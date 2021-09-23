//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////


#include "Others/Console.h"
#include "Application.h"


extern Maple::Application* createApplication();

auto main() -> int32_t
{
	Maple::Console::init();
	Maple::Application::app = createApplication();
	auto retCode = Maple::Application::app->start();
	delete Maple::Application::app;
	return retCode;
}

