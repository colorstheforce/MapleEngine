//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "NativeWindow.h"
#include "WindowWin.h"

namespace Maple 
{

	auto getNativeWindow() -> void* { return nullptr; }

	auto NativeWindow::setTitle(const std::string& title)->void
	{

	}

	auto NativeWindow::newInstance(const WindowInitData& data) ->std::unique_ptr<NativeWindow>
	{
		return std::make_unique<WindowWin>(data);
	}
};

