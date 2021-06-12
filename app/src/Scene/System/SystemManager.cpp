//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Game Engine			                    //

//////////////////////////////////////////////////////////////////////////////

#include "SystemManager.h"

namespace Maple 
{
	auto SystemManager::onUpdate(float dt, Scene* scene) -> void
	{
		for (auto& system : systems)
			system.second->onUpdate(dt, scene);
	}

	auto SystemManager::onImGui()-> void
	{
		for (auto& system : systems)
			system.second->onImGui();
	}
};

