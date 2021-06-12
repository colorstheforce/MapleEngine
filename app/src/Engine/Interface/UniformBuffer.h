//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include <memory>

namespace Maple
{
	class UniformBuffer
	{
	public:
		virtual ~UniformBuffer() {};
		virtual auto setData(uint32_t size, const void* data, uint32_t offset = 0) -> void = 0;
		static auto create(uint32_t size, const void* data)->std::shared_ptr<UniformBuffer>;
	};
};