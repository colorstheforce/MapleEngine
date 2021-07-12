//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanHelper.h"
#include "Others/Console.h"
namespace Maple
{
	VulkanBuffer::VulkanBuffer()
	{
	}

	VulkanBuffer::VulkanBuffer(VkBufferUsageFlags usage, uint32_t size, const void* data)
		:usage(usage),size(size)
	{
		init(usage, size, data);
	}

	VulkanBuffer::~VulkanBuffer()
	{
		release();
	}

	auto VulkanBuffer::init(VkBufferUsageFlags usage, uint32_t size, const void* data) -> void
	{
		//param for creating 
		this->usage = usage;
		this->size = size;
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


		VK_CHECK_RESULT(vkCreateBuffer(*VulkanDevice::get(), &bufferInfo, nullptr, &buffer));

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(*VulkanDevice::get(), buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VulkanHelper::findMemoryType(
			memRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		VK_CHECK_RESULT(vkAllocateMemory(*VulkanDevice::get(), &allocInfo, nullptr, &memory));
		//bind buffer -> 
		vkBindBufferMemory(*VulkanDevice::get(), buffer, memory, 0);
		//if the data is not nullptr, upload the data.
		if (data != nullptr)
			setData(size, data);
	}

	/**
	 * map the data
	 */
	auto VulkanBuffer::map(VkDeviceSize size, VkDeviceSize offset) -> VkResult
	{
		return vkMapMemory(*VulkanDevice::get(), memory, offset, size, 0, &mapped);
	}
	/**
	 * unmap the data.
	 */
	auto VulkanBuffer::unmap() -> void
	{
		if (mapped)
		{
			vkUnmapMemory(*VulkanDevice::get(), memory);
			mapped = nullptr;
		}
	}


	auto VulkanBuffer::flush(VkDeviceSize size, VkDeviceSize offset) -> void
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		VK_CHECK_RESULT(vkFlushMappedMemoryRanges(*VulkanDevice::get(), 1, &mappedRange));
	}

	auto VulkanBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset) -> void
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		VK_CHECK_RESULT(vkInvalidateMappedMemoryRanges(*VulkanDevice::get(), 1, &mappedRange));
	}

	auto VulkanBuffer::setData(uint32_t size, const void * data, uint32_t offset ) -> void
	{
		map(size, offset);
		memcpy(reinterpret_cast<uint8_t*>(mapped) + offset, data, size);
		unmap();
	}

	auto VulkanBuffer::resize(uint32_t size, const void* data) -> void
	{
		release();
		init(usage, size, data);
	}

	auto VulkanBuffer::release() -> void
	{
		if (buffer)
		{
			vkDestroyBuffer(*VulkanDevice::get(), buffer, nullptr);

			if (memory)
			{
				vkFreeMemory(*VulkanDevice::get(), memory, nullptr);
			}
		}
	}
};