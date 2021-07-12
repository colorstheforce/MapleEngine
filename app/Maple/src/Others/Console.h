//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 
////////////////////////////////////////////////////////////////////////////// 

#pragma once
#include <iostream>
#include <string>
#include <stdarg.h>
#include <spdlog/spdlog.h>

namespace Maple 
{
	class Console
	{
	public:
		static auto init() -> void;
		static auto & getLogger() { return logger; }
	private:
		static std::shared_ptr<spdlog::logger> logger;
	};
};

#define LOGV(...)      Maple::Console::getLogger()->trace(__VA_ARGS__)
#define LOGI(...)      Maple::Console::getLogger()->info(__VA_ARGS__)
#define LOGW(...)      Maple::Console::getLogger()->warn(__VA_ARGS__)
#define LOGE(...)      Maple::Console::getLogger()->error(__VA_ARGS__)
#define LOGC(...)      Maple::Console::getLogger()->critical(__VA_ARGS__)



