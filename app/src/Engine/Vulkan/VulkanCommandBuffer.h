//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VulkanHelper.h"
#include "Engine/Interface/CommandBuffer.h"

namespace Maple
{
	class VulkanCommandBuffer;
	class VulkanRenderPass;
	class VulkanFrameBuffer;

	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		VulkanCommandBuffer(bool primary = true);
		~VulkanCommandBuffer();


		virtual auto init(bool primary) -> bool;
		virtual auto init(bool primary,VkCommandPool cmdPool) -> bool;


		virtual auto unload() -> void;
		virtual auto beginRecordingSecondary(VulkanRenderPass* renderPass, VulkanFrameBuffer* framebuffer) -> void;
		virtual auto executeSecondary(VulkanCommandBuffer* primaryCmdBuffer) -> void;


		auto updateViewport(uint32_t width, uint32_t height) -> void override;
		auto beginRecording() -> void override;
		auto endRecording() -> void override;
		auto execute(bool waitFence) -> void override;

		//inline auto getFence() const { return fence; };

		inline auto getCommandBuffer() { return &commandBuffer; }

		autoUnpack(commandBuffer);
		auto executeInternal(VkPipelineStageFlags flags, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, bool waitFence) -> void;
	private:
		VkCommandBuffer commandBuffer = nullptr;
		VkFence fence = nullptr;
		bool primary = false;
	};
};