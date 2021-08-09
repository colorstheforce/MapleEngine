//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 

////////////////////////////////////////////////////////////////////////////// 

#include "VulkanBuffer.h"
#include "Engine/Interface/UniformBuffer.h"

namespace Maple
{
	class VulkanUniformBuffer : public VulkanBuffer, public UniformBuffer
	{
	public:
		VulkanUniformBuffer(uint32_t size, const void* data);
		VulkanUniformBuffer();
		~VulkanUniformBuffer();
		auto init(uint32_t size, const void* data) -> void;
		auto setDynamicData(uint32_t size, uint32_t typeSize, const void* data) -> void;
		auto setData(uint32_t size, const void* data, uint32_t offset = 0) -> void override;
		inline auto getMemory() { return &memory; }
	};
};