//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 

////////////////////////////////////////////////////////////////////////////// 

#pragma once
#include "VulkanHelper.h"
#include <vector>
#include <set>
#include <assert.h>
#include <memory>

namespace Maple
{
	class VulkanCommandPool;

	class VulkanPhysicalDevice final
	{
	public:
		VulkanPhysicalDevice();
		inline operator auto() const { return physicalDevice; }
		inline auto& getProperties() const { return properties; };
		inline auto& getQueueFamilyIndices() const { return indices; }
	private:
		auto isDeviceSuitable(VkPhysicalDevice device) -> bool;
		VkPhysicalDevice physicalDevice;
		VkPhysicalDeviceProperties properties;
		friend class VulkanDevice;
		QueueFamilyIndices indices;
	};


	class VulkanDevice final
	{
	public:
		VulkanDevice();
		~VulkanDevice();
		auto init() -> bool;
		inline const auto getPhysicalDevice() const { return physicalDevice; }
		inline auto getPhysicalDevice() { return physicalDevice; }
		inline auto getGraphicsQueue() { return graphicsQueue; }
		inline auto getPresentQueue() { return presentQueue; }
		inline auto getCommandPool() { return commandPool; }
		inline auto getPipelineCache() const { return pipelineCache; }

		static auto get()->std::shared_ptr<VulkanDevice>;

		inline operator auto() const { return device; }
		static auto release() -> void;
		auto createPipelineCache() -> void;
	private:
		std::shared_ptr<VulkanPhysicalDevice> physicalDevice;
		VkDevice device = nullptr;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		std::shared_ptr<VulkanCommandPool> commandPool;
		static std::shared_ptr<VulkanDevice> instance;


		VkPipelineCache pipelineCache;
	};
};