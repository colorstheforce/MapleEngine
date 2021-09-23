//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Scene/Component/Component.h"
#include <unordered_map>

namespace Maple
{
	class MonoScript;
	class MonoSystem;
	class MAPLE_EXPORT MonoComponent : public Component
	{
	public:
		auto addScript(const std::string& name, MonoSystem * system) -> void;
		inline auto getScripts() const { return scripts; }
		auto remove(const std::string& script) -> void;
	private:
		std::unordered_map<std::string, std::shared_ptr<MonoScript>> scripts;
	};
};