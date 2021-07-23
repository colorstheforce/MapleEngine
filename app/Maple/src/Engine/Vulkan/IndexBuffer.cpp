//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "IndexBuffer.h"
#include "VulkanCommandBuffer.h"

namespace Maple
{
	IndexBuffer::IndexBuffer(const uint16_t* data, uint32_t count)
		: VulkanBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			count * sizeof(uint16_t), data)
		, count(count)
	{
	}

	IndexBuffer::IndexBuffer(const uint32_t* data, uint32_t count)
		: VulkanBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			count * sizeof(uint32_t), data),
		 count(count)
	{
	}

	IndexBuffer::~IndexBuffer()
	{
	}



	auto IndexBuffer::setData(uint32_t size, const void* data) -> void
	{
		if (size != this->size)
		{
			this->size = size;
			VulkanBuffer::resize(size, data);
		}
		else
		{
			VulkanBuffer::setData(size, data);
		}
	}

	auto IndexBuffer::create(uint32_t* data, uint32_t count) ->std::shared_ptr<IndexBuffer>
	{
		return std::make_shared<IndexBuffer>(data, count);
	}

	auto IndexBuffer::bind(CommandBuffer* commandBuffer) const -> void
	{
		vkCmdBindIndexBuffer(*static_cast<VulkanCommandBuffer*>(commandBuffer)
			, buffer, 0, VK_INDEX_TYPE_UINT32);
	}

	auto IndexBuffer::unbind() const -> void
	{
	}

	auto IndexBuffer::getCount() const -> uint32_t
	{
		return count;
	}


	auto IndexBuffer::setCount(uint32_t indexCount) -> void
	{
		count = indexCount;
	}
};