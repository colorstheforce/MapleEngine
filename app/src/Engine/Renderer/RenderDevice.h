//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cstdint>

namespace Maple
{
	class CommandBuffer;
	class RenderDevice
	{
	public:
		virtual ~RenderDevice() = default;
		virtual auto init() -> void = 0;
		virtual auto begin() -> void = 0;
		virtual auto onResize(uint32_t width, uint32_t height) -> void = 0;
		virtual auto present() -> void = 0;
		virtual auto present(CommandBuffer* cmdBuffer) -> void = 0;
	protected:
		uint32_t width;
		uint32_t height;
	};
};
