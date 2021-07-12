//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 

////////////////////////////////////////////////////////////////////////////// 

#pragma once
#include "VulkanHelper.h"
#include "Engine/Interface/SwapChain.h"

namespace Maple
{
#define MAX_SWAPCHAIN_BUFFERS 3

	struct FrameData
	{
		VkSemaphore presentSemaphore;
		VkSemaphore renderSemaphore;
		VkFence		renderFence;
		VkCommandPool commandPool;
		std::shared_ptr<CommandBuffer> commandBuffer;
	};

	class VulkanSwapChain final : public SwapChain
	{
	public:
		VulkanSwapChain();
		~VulkanSwapChain();
		virtual auto init() -> void override;
		autoUnpack(swapChain);

		//	inline auto& getSwapChainImageViews() { return swapChainImageViews; }
		inline auto& getSwapChainImages() { return swapChainImages; }
		inline auto getFormat() { return swapChainImageFormat; }
		inline auto getExtend2D() { return swapChainExtent; }

		auto acquireNextImage(VkSemaphore signalSemaphore)->VkResult;

		auto getCurrentCommandBuffer()->CommandBuffer* override;

		auto present(VkSemaphore waitSemaphore)->VkResult;

		auto getFrameData() -> FrameData&;
		virtual auto resize(uint32_t width, uint32_t height) -> void override;
	private:
		auto createImageView() -> void;
		FrameData frames[MAX_SWAPCHAIN_BUFFERS];

		VkSwapchainKHR swapChain = VK_NULL_HANDLE;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;

		std::vector<VkImage> swapChainImages;

		uint32_t acquireImageIndex = 0;
	};
};