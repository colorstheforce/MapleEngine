//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 

////////////////////////////////////////////////////////////////////////////// 

#include "VulkanSwapChain.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "VulkanCommandBuffer.h"
#include "VulkanHelper.h"
#include "VulkanTexture.h"
#include "Others/Console.h"

#include "Application.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace Maple
{
	VulkanSwapChain::VulkanSwapChain()

	{

	}

	VulkanSwapChain::~VulkanSwapChain()
	{
		vkDestroySwapchainKHR(*VulkanDevice::get(), swapChain, VK_NULL_HANDLE);
	}

	auto VulkanSwapChain::init() -> void
	{
		//find some details 
		SwapChainSupportDetails swapChainSupport = VulkanHelper::querySwapChainSupport(
			*VulkanDevice::get()->getPhysicalDevice(), VulkanContext::get()->getVkSurface());

		VkSurfaceFormatKHR surfaceFormat = VulkanHelper::chooseSwapSurfaceFormat(swapChainSupport.formats);
		
		VkPresentModeKHR presentMode = VulkanHelper::chooseSwapPresentMode(swapChainSupport.presentModes,false);
		
		VkExtent2D extent = VulkanHelper::chooseSwapExtent(swapChainSupport.capabilities);

		uint32_t imageCount = std::min(swapChainSupport.capabilities.maxImageCount,3U);

	

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = VulkanContext::get()->getVkSurface();

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

		auto& indices = VulkanDevice::get()->getPhysicalDevice()->getQueueFamilyIndices();

		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		if (vkCreateSwapchainKHR(*VulkanDevice::get(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(*VulkanDevice::get(), swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(*VulkanDevice::get(), swapChain, &imageCount, swapChainImages.data());

		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
		createImageView();
	}

	auto VulkanSwapChain::createImageView() -> void
	{
		//create imageViews for swapChain;
		for (uint32_t i = 0; i < swapChainImages.size(); i++) {
			auto iv = VulkanHelper::createImageView(swapChainImages[i], swapChainImageFormat, 1, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 1);
			//made it as texture
			swapChainBuffers.emplace_back(std::make_shared<VulkanTexture2D>(
				swapChainImages[i], iv
			));


			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreInfo.pNext = nullptr;

			VkFenceCreateInfo fenceCreateInfo{};
			fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			VK_CHECK_RESULT(vkCreateFence(*VulkanDevice::get(), &fenceCreateInfo, nullptr, &frames[i].renderFence));
			VK_CHECK_RESULT(vkCreateSemaphore(*VulkanDevice::get(), &semaphoreInfo, nullptr, &frames[i].presentSemaphore));
			VK_CHECK_RESULT(vkCreateSemaphore(*VulkanDevice::get(), &semaphoreInfo, nullptr, &frames[i].renderSemaphore));

			VkCommandPoolCreateInfo cmdPoolCI{};

			cmdPoolCI.queueFamilyIndex = VulkanDevice::get()->getPhysicalDevice()->getQueueFamilyIndices().graphicsFamily.value();
			cmdPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

			VK_CHECK_RESULT(vkCreateCommandPool(*VulkanDevice::get(), &cmdPoolCI, nullptr, &frames[i].commandPool));

			frames[i].commandBuffer = CommandBuffer::create(true);
			((VulkanCommandBuffer*)frames[i].commandBuffer.get())->init(true, frames[i].commandPool);

		}
	}

	auto VulkanSwapChain::acquireNextImage(VkSemaphore signalSemaphore) -> VkResult
	{
		return vkAcquireNextImageKHR(*VulkanDevice::get(), swapChain, UINT64_MAX, getFrameData().presentSemaphore, VK_NULL_HANDLE, &acquireImageIndex);
	}

	auto VulkanSwapChain::getCurrentCommandBuffer() -> CommandBuffer* 
	{
		return getFrameData().commandBuffer.get();
	}

	//swap buffer 
	auto VulkanSwapChain::present(VkSemaphore waitSemaphore) -> VkResult
	{

		auto cmdBuffer = ((VulkanCommandBuffer*)(getFrameData().commandBuffer.get()))->getCommandBuffer();

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = VK_NULL_HANDLE;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = cmdBuffer;
		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		submitInfo.pWaitDstStageMask = &waitStage;

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &getFrameData().presentSemaphore;

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &getFrameData().renderSemaphore;
		
		VK_CHECK_RESULT(vkResetFences(*VulkanDevice::get(), 1, &getFrameData().renderFence));
		VK_CHECK_RESULT(vkQueueSubmit(VulkanDevice::get()->getGraphicsQueue(), 1, &submitInfo, getFrameData().renderFence));

		

		VkPresentInfoKHR present;
		present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present.pNext = VK_NULL_HANDLE;
		present.swapchainCount = 1;
		present.pSwapchains = &swapChain;
		present.pImageIndices = &acquireImageIndex;
		present.waitSemaphoreCount = 1;
		present.pWaitSemaphores = &getFrameData().renderSemaphore;
		present.pResults = VK_NULL_HANDLE;
		auto error = vkQueuePresentKHR(VulkanDevice::get()->getPresentQueue(), &present);

		if (error == VK_ERROR_OUT_OF_DATE_KHR)
		{
			LOGE("[Vulkan] Swapchain out of data");
		}
		else if (error == VK_SUBOPTIMAL_KHR)
		{
			LOGE("[Vulkan] Swapchain suboptimal");
		}
		else
		{
			VK_CHECK_RESULT(error);
		}

		currentBuffer = (currentBuffer + 1) % swapChainBuffers.size();

		return error;
	}

	auto VulkanSwapChain::getFrameData() -> FrameData&
	{
		MAPLE_ASSERT(currentBuffer < swapChainBuffers.size(), "invaild swapchain buffer index");
		return frames[currentBuffer];
	}

	auto VulkanSwapChain::resize(uint32_t width, uint32_t height) -> void
	{

	}

};