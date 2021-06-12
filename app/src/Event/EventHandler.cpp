//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "EventHandler.h"
#include "EventDispatcher.h"


namespace Maple 
{
	EventHandler::~EventHandler()
	{
		if (eventDispatcher)
			eventDispatcher->removeEventHandler(this);
	}

	auto EventHandler::remove() -> void
	{
		if (eventDispatcher)
		{
			eventDispatcher->removeEventHandler(this);
			eventDispatcher = nullptr;
		}
	}

};
