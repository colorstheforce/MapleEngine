//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "Mono.h"
#include "MonoSystem.h"
#include "MonoHelper.h"
#include "MonoExporter.h"
#include "MonoScript.h"
#include "Scene/Scene.h"
#include "Others/StringUtils.h"
#include "MonoComponent.h"
#include "MonoScriptInstance.h"

#include "Scene/Component/Transform.h"
#include "Scene/Entity/Entity.h"
#include "Application.h"


namespace Maple
{
	auto MonoSystem::onInit() -> void
	{
		// Get file paths
		const std::string dir_mono_lib = "../Toolchains/mono/lib";
		const std::string dir_mono_etc = "../Toolchains/mono/etc";
		// Point mono to the libs and configuration files
		mono_set_dirs(dir_mono_lib.c_str(), dir_mono_etc.c_str());
		domain = mono_jit_init_version("MapleEngine", "v4.0.30319");
		if (!domain)
		{
			LOGE("mono_jit_init failed");
			return;
		}

		if (!mono_domain_set((MonoDomain*)domain, false))
		{
			LOGE("mono_domain_set failed");
			return;
		}
		// soft debugger needs this
		mono_thread_set_main(mono_thread_current());
	}



	auto MonoSystem::onStart(Scene* scene) -> void
	{
		if (app->getEditorState() == EditorState::Play)
		{
			auto view = scene->getRegistry().view<MonoComponent>();
			for (auto v : view)
			{
				auto& mono = scene->getRegistry().get<MonoComponent>(v);
				for (auto& script : mono.getScripts())
				{
					script.second->onStart(this);
				}
			}
		}
	}


	auto MonoSystem::onUpdate(float dt, Scene* scene)-> void
	{
		if (app->getEditorState() == EditorState::Play) 
		{
			auto view = scene->getRegistry().view<MonoComponent>();
			for (auto v : view)
			{
				auto& mono = scene->getRegistry().get<MonoComponent>(v);
				for (auto & script : mono.getScripts())
				{
					script.second->onUpdate(dt,this);
				}
			}
		}
	}

	auto MonoSystem::onImGui() -> void
	{

	}

	auto MonoSystem::getScript(const uint32_t id) -> MonoScriptInstance*
	{
		if (scripts.find(id) == scripts.end())
			nullptr;

		return scripts[id].get();
	}

	auto MonoSystem::callScriptStart(const MonoScriptInstance* script) -> bool
	{
		if (!script->methodStart || !script->object)
		{
			LOGE("Invalid script");
			return false;
		}

		mono_runtime_invoke(script->methodStart, script->object, nullptr, nullptr);
		return true;
	}

	auto MonoSystem::callScriptUpdate(const MonoScriptInstance* script, float dt) -> bool
	{
		if (!script->methodUpdate || !script->object)
		{
			LOGE("Invalid script instance");
			return false;
		}

		void* args[1];
		args[0] = &dt;

		mono_runtime_invoke(script->methodUpdate, script->object, args, nullptr);
		return true;
	}

	auto MonoSystem::compileSystemAssembly() -> bool
	{
		MonoAssembly* apiAssembly = MonoHelper::compileScript((MonoDomain*)domain, { "Maple.cs" });
		if (!apiAssembly)
		{
			LOGE("Failed to get api assembly {0} : {1}",__FUNCTION__,__LINE__);
			return false;
		}
		MonoImage* callbacksImage = mono_assembly_get_image(apiAssembly);
		if (!callbacksImage)
		{
			LOGE("Failed to get callbacks image");
			return false;
		}
		MonoExporter::exportMono();
		return true;
	}

	MonoSystem::~MonoSystem()
	{
		mono_jit_cleanup((MonoDomain*)domain);
	}

	auto MonoSystem::load(const std::string& name, MonoComponent* component) ->uint32_t
	{
	/*	if (!assemblyCompiled)
		{
			assemblyCompiled = compileSystemAssembly();
			if (!assemblyCompiled)
				return SCRIPT_NOT_LOADED;
		}*/

		const std::string className = StringUtils::getFileNameWithoutExtension(name);
		std::vector<std::string> files;
		auto scene = app->getSceneManager()->getCurrentScene();
		auto view = scene->getRegistry().view<MonoComponent>();
		for (auto v : view)
		{
			auto& mono = scene->getRegistry().get<MonoComponent>(v);
			for (auto& script : mono.getScripts())
			{
				files.emplace_back(script.first);
			}
		}

		files.emplace_back(name);
		auto assembly = MonoHelper::compileScript((MonoDomain*)domain, files,"MapleLibrary.dll");

		if (!assembly)
		{
			LOGE("Failed to Load assembly");
			return SCRIPT_NOT_LOADED;
		}

		auto image = mono_assembly_get_image(assembly);
		if (!image)
		{
			LOGE("Failed to Get image");
			return SCRIPT_NOT_LOADED;
		}

		// Get the class
		auto klass = mono_class_from_name(image, "", className.c_str());
		if (!klass)
		{
			mono_image_close(image);
			LOGE("Failed to get class");
			return SCRIPT_NOT_LOADED;
		}

	
		auto object = mono_object_new((MonoDomain*)domain, klass);
		if (!object)
		{
			mono_image_close(image);
			LOGE("Failed to Create Class Instance");
			return SCRIPT_NOT_LOADED;
		}

		auto methodStart = MonoHelper::getMethod(image, className + ":Start()");
		auto methodUpdate = MonoHelper::getMethod(image, className + ":Update(single)");
		auto& script = std::make_shared<MonoScriptInstance>();
		scripts[++scriptId] = script;
		script->methodStart = methodStart;
		script->methodUpdate = methodUpdate;
		script->object = object;
		script->klass = klass;
		script->assembly = assembly;
		script->image = image;
		auto entity = component->getEntity();
	
		// Set entity handle
		if (!script->setValue(&component->getEntityId(), "_internal_entity_handle"))
		{
			mono_image_close(image);
			LOGE("Failed to set entity handle");
			return SCRIPT_NOT_LOADED;
		}

		if (!script->setValue(entity.tryGetComponent<Transform>(), "_internal_transform_handle"))
		{
			mono_image_close(image);
			LOGE("Failed to set transform handle");
			return SCRIPT_NOT_LOADED;
		}

		// Call the default constructor
		mono_runtime_object_init(object);
		if (!object)
		{
			mono_image_close(image);
			LOGE("Failed to run class constructor");
			return SCRIPT_NOT_LOADED;
		}
		return scriptId;
	}

};