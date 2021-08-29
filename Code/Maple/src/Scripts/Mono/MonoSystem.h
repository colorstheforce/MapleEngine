//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Scene/System/ISystem.h"
#include "Event/EventHandler.h"
#include <unordered_map>
#include <memory>

namespace Maple 
{
	class Scene;
	class MonoComponent;
	struct MonoScriptInstance;


	static const uint32_t SCRIPT_NOT_LOADED = 0;

	class MonoSystem final : public ISystem
	{
	public:
		~MonoSystem();
		auto onInit() -> void override;
		auto onUpdate(float dt, Scene* scene) -> void override;
		auto onImGui() -> void override;

		auto onStart(Scene* scene) -> void;
		auto getScript(const uint32_t id) ->MonoScriptInstance*;
		auto callScriptStart(const MonoScriptInstance* script) -> bool;
		auto callScriptUpdate(const MonoScriptInstance* script,float dt) -> bool;
		auto load(const std::string& name, MonoComponent* component)->uint32_t;

	private:
		auto compileSystemAssembly() -> bool;

		std::unordered_map<uint32_t, std::shared_ptr<MonoScriptInstance>> scripts;
		uint32_t scriptId = SCRIPT_NOT_LOADED;
		bool assemblyCompiled = false;

		EventHandler handler;
	};
};