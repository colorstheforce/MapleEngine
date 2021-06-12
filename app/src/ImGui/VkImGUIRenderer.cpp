//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "VkImGUIRenderer.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/VulkanSwapChain.h"
#include "Engine/Vulkan/VulkanDevice.h"
#include "Engine/Vulkan/VulkanRenderPass.h"
#include "Engine/Vulkan/VulkanCommandBuffer.h"
#include "Engine/Vulkan/VulkanTexture.h"
#include "Engine/Vulkan/VulkanFrameBuffer.h"

#include <imgui_impl_vulkan.h>
#include "Application.h"
#include "Engine/Renderer/VkRenderDevice.h"

static ImGui_ImplVulkanH_WindowData g_WindowData;
static VkAllocationCallbacks* g_Allocator = nullptr;
static VkDescriptorPool g_DescriptorPool = VK_NULL_HANDLE;
static void checkVKResult(VkResult err)
{
	if (err == 0) return;
	printf("VkResult %d\n", err);
	if (err < 0)
		abort();
}

namespace Maple
{
	VkImGUIRenderer::VkImGUIRenderer(uint32_t width, uint32_t height, bool clearScreen)
		:width(width), height(height), clearScreen(clearScreen)
	{

	}

	VkImGUIRenderer::~VkImGUIRenderer()
	{
		for (int i = 0; i < VulkanContext::get()->getSwapChain()->getSwapChainBuffers().size(); i++)
		{
			ImGui_ImplVulkanH_FrameData* fd = &g_WindowData.Frames[i];
			vkDestroyFence(*VulkanDevice::get(), fd->Fence, g_Allocator);
			vkDestroyCommandPool(*VulkanDevice::get(), fd->CommandPool, g_Allocator);
		}
		vkDestroyDescriptorPool(*VulkanDevice::get(), g_DescriptorPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
	}

	auto VkImGUIRenderer::setupVulkanWindowData(ImGui_ImplVulkanH_WindowData* wd, VkSurfaceKHR surface, int32_t width, int32_t height) -> void
	{
		//some uniforms in vulkan shader.
		const uint32_t maxSize = 10;
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, maxSize },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxSize },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, maxSize },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, maxSize },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, maxSize },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, maxSize },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxSize },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, maxSize },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, maxSize },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, maxSize },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, maxSize }
		};
		VkDescriptorPoolCreateInfo poolnfo = {};
		poolnfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolnfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolnfo.maxSets = maxSize * IM_ARRAYSIZE(poolSizes);
		poolnfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
		poolnfo.pPoolSizes = poolSizes;
		VkResult err = vkCreateDescriptorPool(*VulkanDevice::get(), &poolnfo, g_Allocator, &g_DescriptorPool);
		checkVKResult(err);

		wd->Surface = surface;
		wd->ClearEnable = clearScreen;

		auto bufferCount = VulkanContext::get()->getSwapChain()->getSwapChainBuffers().size();

		for (int i = 0; i < bufferCount; i++)
		{
			auto commandBuffer = std::make_shared<VulkanCommandBuffer>();
			commandBuffer->init(true);
			commandBuffers[i] = commandBuffer;
		}

		auto swapChain = std::static_pointer_cast<VulkanSwapChain>(VulkanContext::get()->getSwapChain());
		wd->Swapchain = *swapChain;
		wd->Width = width;
		wd->Height = height;

		wd->BackBufferCount = bufferCount;

		AttachmentInfo textureTypes[2] =
		{
			{ TextureType::COLOR, TextureFormat::BGRA8, true}
		};

		RenderPassInfo renderPassInfo;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.textureType = textureTypes;
		renderPassInfo.clear = clearScreen;

		renderPass = std::make_shared<VulkanRenderPass>(renderPassInfo);
		wd->RenderPass = *renderPass;

		// Create The Image Views
		{
			for (uint32_t i = 0; i < wd->BackBufferCount; i++)
			{
				auto scBuffer = (VulkanTexture2D*)swapChain->getTexture(i).get();
				wd->BackBuffer[i] = scBuffer->getImage();
				wd->BackBufferView[i] = scBuffer->getImageView();
			}
		}

		TextureType attachmentTypes[1];
		attachmentTypes[0] = TextureType::COLOR;


		FrameBufferInfo bufferInfo{};
		bufferInfo.width = wd->Width;
		bufferInfo.height = wd->Height;
		bufferInfo.attachments.resize(1);

		bufferInfo.renderPass = renderPass;
		bufferInfo.types = {
			TextureType::COLOR
		};
		bufferInfo.screenFBO = true;
		frameBuffers.resize(bufferCount);
		for (uint32_t i = 0; i < bufferCount; i++)
		{
			bufferInfo.attachments[0] = swapChain->getTexture(i);
			frameBuffers[i] = std::make_shared<VulkanFrameBuffer>(bufferInfo);
			wd->Framebuffer[i] = *frameBuffers[i];
		}

	}
	auto VkImGUIRenderer::init() -> void
	{

		ImGui_ImplVulkanH_WindowData* wd = &g_WindowData;
		VkSurfaceKHR surface = VulkanContext::get()->getVkSurface();
		setupVulkanWindowData(wd, surface, width, height);

		// Setup Vulkan binding
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = VulkanContext::get()->getVkInstance();
		init_info.PhysicalDevice = *VulkanDevice::get()->getPhysicalDevice();
		init_info.Device = *VulkanDevice::get();
		init_info.QueueFamily = VulkanDevice::get()->getPhysicalDevice()->getQueueFamilyIndices().graphicsFamily.value();
		init_info.Queue = VulkanDevice::get()->getGraphicsQueue();
		init_info.PipelineCache = VulkanDevice::get()->getPipelineCache();
		init_info.DescriptorPool = g_DescriptorPool;
		init_info.Allocator = g_Allocator;
		init_info.CheckVkResultFn = NULL;
		ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);
		// Upload Fonts

		rebuildFontTexture();

	}
	auto VkImGUIRenderer::newFrame() -> void
	{
	}
	auto VkImGUIRenderer::render(VulkanCommandBuffer* commandBuffer) -> void
	{
		frameRender(&g_WindowData);
	}
	auto VkImGUIRenderer::onResize(uint32_t width, uint32_t height) -> void
	{
		auto swapChain = std::static_pointer_cast<VulkanSwapChain>(VulkanContext::get()->getSwapChain());
		auto* wd = &g_WindowData;
		wd->Swapchain = *swapChain;
		for (uint32_t i = 0; i < wd->BackBufferCount; i++)
		{
			auto scBuffer = (VulkanTexture2D*)swapChain->getTexture(i).get();
			wd->BackBuffer[i] = scBuffer->getImage();
			wd->BackBufferView[i] = scBuffer->getImageView();
		}

		wd->Width = width;
		wd->Height = height;

		frameBuffers.clear();
		frameBuffers.resize(swapChain->getSwapChainBuffers().size());


		// Create Framebuffer

		FrameBufferInfo bufferInfo{};
		bufferInfo.width = wd->Width;
		bufferInfo.height = wd->Height;
		bufferInfo.renderPass = renderPass;
		bufferInfo.types = { TextureType::COLOR };
		bufferInfo.screenFBO = true;
		bufferInfo.attachments.resize(1);

		for (uint32_t i = 0; i < swapChain->getSwapChainBuffers().size(); i++)
		{
			bufferInfo.attachments[0] = swapChain->getTexture(i);
			frameBuffers[i] = std::make_shared<VulkanFrameBuffer>(bufferInfo);
			wd->Framebuffer[i] = *frameBuffers[i];
		}
	}
	auto VkImGUIRenderer::clear() -> void
	{
	}
	auto VkImGUIRenderer::frameRender(ImGui_ImplVulkanH_WindowData* wd) -> void
	{
		wd->FrameIndex = VulkanContext::get()->getSwapChain()->getCurrentBuffer();

		auto cmd = VulkanContext::get()->getSwapChain()->getCurrentCommandBuffer();

		renderPass->beginRenderPass(cmd, { 0.1f, 0.1f, 0.1f, 1.0f }, frameBuffers[wd->FrameIndex].get(), SubPassContents::INLINE, wd->Width, wd->Height);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), 
			(*(VulkanCommandBuffer*)cmd)
		);

		renderPass->endRenderpass(cmd);

		//app->getRenderDevice()->present(commandBuffers[wd->FrameIndex].get());
	}

	auto VkImGUIRenderer::rebuildFontTexture() -> void
	{
		ImGuiIO& io = ImGui::GetIO();

		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		fontTexture = std::make_shared<VulkanTexture2D>(width, height, pixels, TextureParameters(TextureFilter::NEAREST, TextureFilter::NEAREST));

		VkWriteDescriptorSet write_desc[1] = {};
		write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_desc[0].dstSet = ImGui_ImplVulkanH_GetFontDescriptor();
		write_desc[0].descriptorCount = 1;
		write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write_desc[0].pImageInfo = fontTexture->getDescriptor();
		vkUpdateDescriptorSets(*VulkanDevice::get(), 1, write_desc, 0, nullptr);

		io.Fonts->TexID = (ImTextureID)fontTexture->getHandle();// GetImage();

		ImGui_ImplVulkan_AddTexture(io.Fonts->TexID, ImGui_ImplVulkanH_GetFontDescriptor());

		ImGui_ImplVulkan_InvalidateFontUploadObjects();
	}
};
