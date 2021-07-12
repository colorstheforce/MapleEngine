//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Engine/Vulkan/VulkanHelper.h"
#include <memory>
#include <vector>

struct ImGui_ImplVulkanH_Window;
namespace Maple 
{

	class VulkanCommandBuffer;
	class VulkanFrameBuffer;
	class VulkanRenderPass;
	class VulkanTexture2D;


	//a class for renderering imgui in vulkan

	class VkImGUIRenderer 
	{
	public:
		VkImGUIRenderer(uint32_t width, uint32_t height, bool clearScreen);
		~VkImGUIRenderer();

		auto init() -> void;
		auto newFrame() -> void;
		auto render(VulkanCommandBuffer* commandBuffer) -> void;
		auto onResize(uint32_t width, uint32_t height) -> void;
		auto clear() -> void;
		
		auto frameRender(ImGui_ImplVulkanH_Window* wd) -> void;
		auto setupVulkanWindowData(ImGui_ImplVulkanH_Window* wd,VkSurfaceKHR surface, int32_t width, int32_t height) -> void;
		auto rebuildFontTexture() -> void;
	private:
		void* windowHandle = nullptr;
	
		std::shared_ptr<VulkanCommandBuffer> commandBuffers[3];
		std::vector<std::shared_ptr<VulkanFrameBuffer>> frameBuffers;

		std::shared_ptr<VulkanRenderPass> renderPass;
		std::shared_ptr<VulkanTexture2D> fontTexture;
		


		uint32_t width;
		uint32_t height;
		bool clearScreen;
	};
	
};