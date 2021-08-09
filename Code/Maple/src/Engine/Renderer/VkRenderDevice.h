//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cstdint>
#include "Engine/Vulkan/VulkanHelper.h"
#include "RenderDevice.h"
#define NUM_SEMAPHORES 10

namespace Maple
{
	class VkRenderDevice : public RenderDevice
	{
	public:
		VkRenderDevice() = default;
		VkRenderDevice(uint32_t width, uint32_t height);
		~VkRenderDevice();
		auto init() -> void override;
		auto begin() -> void override;
		auto onResize(uint32_t width, uint32_t height) -> void override;
		auto present() -> void override;
		auto end() -> void override;
		auto present(CommandBuffer* cmdBuffer) -> void override;
		auto acquireNextImage() -> void;
	private:
		auto createSemaphores() -> void;
		VkSemaphore imageAvailableSemaphore[NUM_SEMAPHORES];
		uint32_t currentSemaphoreIndex = 0;
	};
};
