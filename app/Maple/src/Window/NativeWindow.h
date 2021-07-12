//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>


namespace Maple 
{
	struct WindowInitData
	{
		uint32_t width;
		uint32_t height;
		bool vsync;
		std::string title;
	};


	class NativeWindow
	{
	public:
		NativeWindow() = default;
		virtual ~NativeWindow() {}

		virtual auto onUpdate() -> void = 0;
		virtual auto getWidth() const -> uint32_t = 0;
		virtual auto getHeight() const->uint32_t = 0;
		virtual auto setVSync(bool sync) -> void = 0;
		virtual auto isVSync() const -> bool = 0;
		virtual auto getNativeInterface() -> void* = 0;
		virtual auto getNativeWindow() -> void* { return nullptr; }
		virtual auto setTitle(const std::string& title)->void;

		inline auto getExtensions() const { return extensions; };

		static auto newInstance(const WindowInitData & data)->std::unique_ptr<NativeWindow>;
		virtual auto init() -> void = 0;
	protected:
		std::vector<const char*> extensions;
	private:
		
	};


};