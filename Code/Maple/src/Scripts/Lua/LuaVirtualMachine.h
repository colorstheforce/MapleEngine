//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Engine/Core.h"
#include <string>

struct lua_State;

namespace Maple 
{
	class MAPLE_EXPORT LuaVirtualMachine final
	{
	public:
		LuaVirtualMachine();
		~LuaVirtualMachine();
		auto init() -> void;
		inline auto getState() { return L; }
	private:
		auto addSystemPath(const std::string& path) -> void;
		auto addPath(const std::string& path) -> void;
		lua_State * L = nullptr;
	};
};