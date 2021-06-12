
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VulkanBuffer.h"


namespace Maple
{
	class CommandBuffer;
	class Pipeline;
   /**
	* VertexBuffer for Vulkan
	*/
	class VertexBuffer : public VulkanBuffer
	{
	public:
		VertexBuffer();
		VertexBuffer(const BufferUsage & usage);
		~VertexBuffer();

		/**
		 * set data 
		 */
		auto setData(uint32_t size, const void* data) -> void;
		auto setSubData(uint32_t size, const void* data, uint32_t offset) -> void;
		auto releasePointer() -> void;
		/**
		 * bind to cmd 
		 */
		auto bind(CommandBuffer* commandBuffer, Pipeline* pipeline) -> void;
		auto unbind() -> void;
		auto getPointer() -> void*;
	protected:
		bool mappedBuffer = false;
		BufferUsage bufferUsage = BufferUsage::STATIC;
	};
};