//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////


#include "Editor/Editor.h"

#include "Others/Console.h"
auto main() -> int32_t
{
	Maple::Console::init();

	//app = new Maple::Application();

	app = new Maple::Editor();

	return app->start();
}

