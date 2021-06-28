//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "VertexBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
namespace Maple
{
	VertexBuffer::VertexBuffer()
	{
		this->usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}

	VertexBuffer::VertexBuffer(const BufferUsage& usage)
		:bufferUsage(usage)
	{
		this->usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}

	VertexBuffer::~VertexBuffer()
	{
	}


	auto VertexBuffer::setData(uint32_t size, const void* data) -> void
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

	auto VertexBuffer::setSubData(uint32_t size, const void* data, uint32_t offset) -> void
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

	auto VertexBuffer::releasePointer() -> void
	{
		if (mappedBuffer)
		{
			VulkanBuffer::flush(size);
			VulkanBuffer::unmap();
			mappedBuffer = false;
		}
	}

	auto VertexBuffer::bind(CommandBuffer* commandBuffer, Pipeline* pipeline) -> void
	{
		VkDeviceSize offsets[1] = { 0 };
		if (commandBuffer)
			vkCmdBindVertexBuffers(*static_cast<VulkanCommandBuffer*>(commandBuffer), 0, 1, &buffer, offsets);

	}

	auto VertexBuffer::unbind() -> void
	{
	}

	auto VertexBuffer::getPointer() -> void*
	{
		if (!mappedBuffer)
		{
			VulkanBuffer::map();
			mappedBuffer = true;
		}
		return mapped;
	}

	auto VertexBuffer::create(const BufferUsage& usage) ->std::shared_ptr<VertexBuffer>
	{
		return std::make_shared<VertexBuffer>(usage);
	}

};