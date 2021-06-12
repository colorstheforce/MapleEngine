
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "VkRenderDevice.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/VulkanDevice.h"
#include "Engine/Vulkan/VulkanSwapChain.h"
#include "Engine/Vulkan/VulkanCommandBuffer.h"
#include "Others/Console.h"
#include "Application.h"


namespace Maple
{
	VkRenderDevice::VkRenderDevice(uint32_t width, uint32_t height)
	{
		this->width = width;
		this->height = height;
	}

	VkRenderDevice::~VkRenderDevice()
	{
		/*for (int i = 0; i < NUM_SEMAPHORES; i++)
		{
			vkDestroySemaphore(*VulkanDevice::get(),imageAvailableSemaphore[i], nullptr);
		}*/
	}

	auto VkRenderDevice::init() -> void
	{
		auto nativeWindow = app->getWindow()->getNativeInterface();
		VulkanContext::get()->init();
		VulkanContext::get()->createSurface((GLFWwindow*)nativeWindow);
		VulkanDevice::get()->init();
		VulkanContext::get()->createSwapChain();
		createSemaphores();
	}

	auto VkRenderDevice::begin() -> void
	{
		acquireNextImage();
		VulkanContext::get()->getSwapChain()->getCurrentCommandBuffer()->beginRecording();
	}


	auto VkRenderDevice::acquireNextImage() -> void
	{
		auto vkSwapchian = std::static_pointer_cast<VulkanSwapChain>(VulkanContext::get()->getSwapChain());
		auto result = vkSwapchian->acquireNextImage(nullptr);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			onResize(width, height);
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}
	}

	auto VkRenderDevice::onResize(uint32_t width, uint32_t height) -> void
	{
		if (width == 0 || height == 0)
			return;

		this->width = width;
		this->height = height;

		LOGI("W : {0}, H : {1}", width, height);

		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			*VulkanDevice::get()->getPhysicalDevice(), 
			VulkanContext::get()->getVkSurface(), &capabilities);

		this->width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width,width));
		this->height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, height));

		VulkanContext::get()->resize(width, height);
	}


	auto VkRenderDevice::present() -> void
	{
		auto vkSwapChain = std::static_pointer_cast<VulkanSwapChain>(VulkanContext::get()->getSwapChain());
		vkSwapChain->getCurrentCommandBuffer()->endRecording();
		auto result = vkSwapChain->present(nullptr);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			onResize(width, height);
		}
	}	


	auto VkRenderDevice::present(CommandBuffer * cmdBuffer) -> void
	{

		/*static_cast<VulkanCommandBuffer*>(cmdBuffer)->executeInternal(
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			imageAvailableSemaphore[currentSemaphoreIndex],
			imageAvailableSemaphore[currentSemaphoreIndex + 1],
			true);*/

	}


	auto VkRenderDevice::createSemaphores() -> void
	{
	/*	VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.pNext = nullptr;
		for (int i = 0; i < NUM_SEMAPHORES; i++)
		{
			VK_CHECK_RESULT(vkCreateSemaphore(*VulkanDevice::get(), &semaphoreInfo, nullptr, &imageAvailableSemaphore[i]));
		}*/
	}

};


