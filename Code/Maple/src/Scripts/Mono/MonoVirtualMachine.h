//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Engine/Core.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <functional>
#include "Mono.h"

namespace Maple
{
	class MAPLE_EXPORT MonoVirtualMachine final
	{
	public:
		MonoVirtualMachine();
		~MonoVirtualMachine();

		auto compileAssembly(const std::function<void(void*)> & callback) -> void;

		auto loadAssembly(const std::string & path, const std::string & name) -> std::shared_ptr<MapleMonoAssembly>;
		auto initializeAssembly(std::shared_ptr<MapleMonoAssembly> assembly) -> void;
		auto unloadScriptDomain() -> void;


		auto init() -> void;
		inline auto getDomain() { return scriptDomain; }
		inline auto getAssembly(const std::string& name) { return assemblies[name]; };

		auto findClass(const std::string& ns, const std::string& type)->std::shared_ptr<MapleMonoClass>;
		auto findClass(MonoClass* clazz)->std::shared_ptr<MapleMonoClass>;
		//sure,it is a singleton. however, it is managed by application
		static auto get()->std::shared_ptr< MonoVirtualMachine>;
	private:
		MonoDomain* domain = nullptr;
		MonoDomain* scriptDomain = nullptr;
		std::shared_ptr<MapleMonoAssembly> corlibAssembly;

		std::unordered_map<std::string, std::shared_ptr<MapleMonoAssembly>> assemblies;
		std::unordered_map<std::string, 
			std::vector<std::shared_ptr<MonoScriptInstance>>
		> scriptInstances;

	};
};