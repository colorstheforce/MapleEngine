//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "UniformBuffer.h"
#include "Engine/Vulkan/VulkanUniformBuffer.h"

namespace Maple 
{

	auto UniformBuffer::create(uint32_t size, const void* data) ->std::shared_ptr<UniformBuffer>
	{
		return std::make_shared<VulkanUniformBuffer>(size,data);
	}

};