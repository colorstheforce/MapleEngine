//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <memory>
#include "VulkanBuffer.h"

namespace Maple 
{
	class CommandBuffer;

	/**
	 * IndexBuffer for Vulkan 
	 */
	class IndexBuffer : public VulkanBuffer
	{
	public:
		IndexBuffer(const uint16_t* data, uint32_t count);
		IndexBuffer(const uint32_t* data, uint32_t count);
		~IndexBuffer();

		/**
		 * bind to commandBuffer
		 * 
		 */
		auto bind(CommandBuffer* commandBuffer) const -> void;
		auto unbind() const -> void;
		auto getCount() const->uint32_t;
		auto setCount(uint32_t indexCount) -> void;

		auto setData(uint32_t size, const void* data) -> void;

		static auto create(uint32_t* data, uint32_t count)->std::shared_ptr<IndexBuffer>;
	private:
		uint32_t count;
	};
};

