//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once


struct lua_State;


namespace Maple
{
	namespace InputExport 
	{
		auto exportLua(lua_State* L) -> void;
	};
};


