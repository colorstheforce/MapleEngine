//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine									//
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

#include "Mono.h"

namespace Maple
{
	class MAPLE_EXPORT  MapleMonoAssembly
	{
	public:
		struct ClassId
		{
			struct Hash
			{
				size_t operator()(const ClassId& v) const;
			};

			struct Equals
			{
				inline auto operator()(const ClassId& a, const ClassId& b) const { return a.name == b.name && a.namespaceName == b.namespaceName && a.genericInstance == b.genericInstance; }
			};

			ClassId(const std::string& namespaceName, std::string name, MonoClass* genericInstance = nullptr);

			std::string namespaceName;
			std::string name;
			MonoClass* genericInstance;
		};

	public:
		virtual ~MapleMonoAssembly();

		inline auto& getName() const { return name; }
		auto getClass(const std::string& namespaceName, const std::string& name) const ->std::shared_ptr<MapleMonoClass>;
		auto getClass(MonoClass* rawMonoClass) const->std::shared_ptr<MapleMonoClass>;
		auto getAllClasses() const -> const std::vector<std::shared_ptr<MapleMonoClass>>&;

		/**
		 * Invokes a zero-parameter static method in the format "Class::Method". Used primarily for invoking an assembly
		 * entry point.
		 */
		auto invoke(const std::string& functionName) -> void;
		MapleMonoAssembly(const std::string& path, const std::string& name);
	private:
		friend class MonoVirtualMachine;

	

		auto getClass(const std::string& namespaceName, const std::string& name, MonoClass* rawMonoClass) const->std::shared_ptr<MapleMonoClass>;
		auto load() -> void;
		auto loadFromImage(MonoImage* image) -> void;
		auto unload() -> void;
		auto isGenericClass(const std::string& name) const -> bool;

		std::string name;
		std::string path;
		MonoImage* monoImage = nullptr;
		MonoAssembly* monoAssembly = nullptr;
		uint8_t * debugData = nullptr;
		bool loaded = false;
		bool dependency = false;

		mutable std::unordered_map<ClassId, std::shared_ptr<MapleMonoClass>, ClassId::Hash, ClassId::Equals> classes;
		mutable std::unordered_map<MonoClass*, std::shared_ptr<MapleMonoClass>> classesByRaw;

		mutable bool haveCachedClassList = false;
		mutable std::vector<std::shared_ptr<MapleMonoClass>> cachedClassList;
	};

};