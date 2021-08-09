//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 
////////////////////////////////////////////////////////////////////////////// 


#pragma once
#include <memory>

namespace Maple
{
	class CommandBuffer
	{
	public:
		static auto create(bool primary)->std::shared_ptr<CommandBuffer>;

		virtual auto execute(bool waitFence)-> void = 0;
		virtual auto endRecording() -> void = 0;
		virtual auto beginRecording() -> void = 0;
		virtual auto updateViewport(uint32_t width, uint32_t height) -> void = 0;

	};
};