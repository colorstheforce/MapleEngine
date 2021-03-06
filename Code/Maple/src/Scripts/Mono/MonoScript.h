//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include <cstdint>
#include <memory>
#include "Engine/Core.h"

namespace Maple 
{
	class MonoSystem;
	class MonoComponent;
	class MapleMonoObject;
	class MapleMonoMethod;

	class MAPLE_EXPORT MonoScript final
	{
	public:
		MonoScript(const std::string & name, MonoComponent * component, MonoSystem* system);
		~MonoScript();
		auto onStart(MonoSystem* system) -> void;
		auto onUpdate(float dt,MonoSystem * system) -> void;
		inline auto getClassName() const { return className; }
		inline auto getClassNameInEditor() const { return classNameInEditor; }
		auto loadFunction() -> void;
	private:

		MonoComponent* component = nullptr;
		uint32_t id = 0;
		std::string name;
		std::string className;
		std::string classNameInEditor;
		std::shared_ptr<MapleMonoObject> scriptObject;

		std::shared_ptr<MapleMonoMethod> updateFunc;
		std::shared_ptr<MapleMonoMethod> startFunc;
		std::shared_ptr<MapleMonoMethod> destoryFunc;
	};
};