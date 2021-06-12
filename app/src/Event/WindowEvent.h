//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Event.h"

namespace Maple
{
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(int32_t width, int32_t height);
		inline auto getWidth() const { return width; }
		inline auto getHeight() const { return height; }
		GENERATE_EVENT_CLASS_TYPE(WindowResize);
	private:
		int32_t width,height;
	};
};