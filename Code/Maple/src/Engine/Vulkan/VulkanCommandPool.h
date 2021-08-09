//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "VulkanHelper.h"
namespace Maple
{
	class VulkanCommandPool
	{
	public:
		VulkanCommandPool();
		~VulkanCommandPool();

		auto init() -> void;
		autoUnpack(commandPool);
	private:
		VkCommandPool commandPool;
	};
};