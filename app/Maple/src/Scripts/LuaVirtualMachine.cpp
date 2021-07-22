//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
extern "C" {
# include "lua.h"
# include "lauxlib.h"
# include "lualib.h"
}
#include "LuaVirtualMachine.h"
#include "Others/Console.h"
#include <LuaBridge/LuaBridge.h>
#include <functional>
#include "LuaComponent.h"
#include "Devices/InputExport.h"
#include "MathExport.h"
#include "Devices/Input.h"
#include "ComponentExport.h"
#include <filesystem>

namespace Maple
{
	LuaVirtualMachine::LuaVirtualMachine()
	{

	}

	LuaVirtualMachine::~LuaVirtualMachine()
	{
		lua_close(L);
	}

	auto LuaVirtualMachine::init() -> void
	{
		L = luaL_newstate();
		luaL_openlibs(L);//load all default lua functions
		addPath(".");
		InputExport::exportLua(L);
		LogExport::exportLua(L);
		MathExport::exportLua(L);
		ComponentExport::exportLua(L);
	}


	auto LuaVirtualMachine::addSystemPath(const std::string& path) -> void
	{
		std::string v;
		lua_getglobal(L, "package");
		lua_getfield(L, -1, "path");
		v.append(lua_tostring(L, -1));
		v.append(";");
		v.append(path.c_str());
		lua_pushstring(L, v.c_str());
		lua_setfield(L, -3, "path");
		lua_pop(L, 2);
	}

	auto LuaVirtualMachine::addPath(const std::string & path) -> void
	{
		addSystemPath(path + "/?.lua");

		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			bool isDir = std::filesystem::is_directory(entry);
			if (isDir) {
				addPath(entry.path().string());
			}
		}
	}

};
