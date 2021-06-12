//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "NativeWindow.h"

#include <memory>

#ifdef PLATFORM_DESKTOP

struct GLFWwindow;

namespace Maple 
{
	class WindowWin : public NativeWindow
	{
	public:
		WindowWin(const WindowInitData& data);
		virtual ~WindowWin() {}
		auto onUpdate() -> void override;
		auto setVSync(bool sync) -> void override;
		auto getNativeWindow() -> void* override;
		inline auto isVSync() const -> bool override {return data.vsync;}
		inline auto getWidth() const -> uint32_t override { return data.width; };
		inline auto getHeight() const->uint32_t  override { return data.height; };
		inline auto getNativeInterface() -> void* override { return nativeInterface; }
		auto init() -> void override;

		auto setTitle(const std::string& title)->void override;

	private:
		auto registerNativeEvent(const WindowInitData& data) -> void;

		WindowInitData  data;
		GLFWwindow* nativeInterface = nullptr;
	};

};
#endif // PLATFORM_DESKTOP
