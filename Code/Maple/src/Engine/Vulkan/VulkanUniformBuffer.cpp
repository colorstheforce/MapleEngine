//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "VulkanUniformBuffer.h"
#include <memory.h>
namespace Maple
{

	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size, const void* data)
	{
		VulkanBuffer::init(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, size, data);
	}

	VulkanUniformBuffer::VulkanUniformBuffer()
	{
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
	}

	auto VulkanUniformBuffer::init(uint32_t size, const void* data) -> void
	{
		VulkanBuffer::init(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, size, data);
	}

	auto VulkanUniformBuffer::setDynamicData(uint32_t size, uint32_t typeSize, const void* data) -> void
	{
		VulkanBuffer::map();
		memcpy(mapped, data, size);
		VulkanBuffer::flush(size);
		VulkanBuffer::unmap();
	}

	auto VulkanUniformBuffer::setData(uint32_t size, const void* data, uint32_t offset /*= 0*/) -> void
	{
		VulkanBuffer::setData(size, data, offset);
	}

};