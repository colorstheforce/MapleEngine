//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "LuaSystem.h"
#include "Scene/Scene.h"
#include "LuaComponent.h"
#include "Application.h"

namespace Maple
{
	auto LuaSystem::onInit() -> void
	{
		
	}

	auto LuaSystem::onUpdate(float dt, Scene* scene)-> void
	{
		if (Application::get()->getEditorState() == EditorState::Play) 
		{
			auto view = scene->getRegistry().view<LuaComponent>();
			for (auto v : view)
			{
				auto& lua = scene->getRegistry().get<LuaComponent>(v);
				lua.onUpdate(dt);
			}
		}
	}

	auto LuaSystem::onImGui() -> void
	{

	}
};