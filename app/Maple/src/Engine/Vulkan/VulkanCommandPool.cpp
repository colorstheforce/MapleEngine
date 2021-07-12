//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "Others/Console.h"
namespace Maple
{
	VulkanCommandPool::VulkanCommandPool()
	{
		init();
	}

	VulkanCommandPool::~VulkanCommandPool()
	{
		vkDestroyCommandPool(*VulkanDevice::get(), commandPool, nullptr);
	}

	auto VulkanCommandPool::init() -> void
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		//poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = VulkanDevice::get()->getPhysicalDevice()->getQueueFamilyIndices().graphicsFamily.value();

		VK_CHECK_RESULT(vkCreateCommandPool(*VulkanDevice::get(), &poolInfo, nullptr, &commandPool));
	}
};