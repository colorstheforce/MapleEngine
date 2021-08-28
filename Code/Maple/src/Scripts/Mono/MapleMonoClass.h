//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine									//
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Mono.h"
#include <string>
#include <unordered_map>
#include <memory>

namespace Maple
{
	class MapleMonoClass final
	{
		struct MethodId
		{
			struct Hash
			{
				auto operator()(const MethodId& v) const ->size_t;
			};

			struct Equals
			{
				inline auto operator()(const MethodId& a, const MethodId& b) const { return a.name == b.name && a.numParams == b.numParams; }
			};

			MethodId(const std::string & name, uint32_t numParams);
			std::string name;
			uint32_t numParams;
		};

	public:
		MapleMonoClass(const std::string& nameSpace, const std::string & type, MonoClass* monoClass, const MapleMonoAssembly* parentAssembly);
		~MapleMonoClass();

		inline auto& getNamespace() const { return nameSpace; }
		inline auto& getTypeName() const { return typeName; }
		inline auto& getFullName() const { return fullName; }

		 
		auto getMethod(const std::string& name, uint32_t numParams = 0) const ->std::shared_ptr<MapleMonoMethod>;
		auto getField(const std::string& name) const ->std::shared_ptr<MapleMonoField>;
		auto getProperty(const std::string& name) const ->std::shared_ptr<MapleMonoProperty>;

		auto getAttribute(MapleMonoClass* monoClass) const ->MonoObject*;
		auto getBaseClass() const ->std::shared_ptr < MapleMonoClass>;

		/**
		 * Returns an object referencing a method, expects exact method name with parameters.
		 *
		 * @note
		 * Does not query base class methods.
		 * Returns null if method cannot be found.
		 * Example: name = "CreateInstance", signature = "Vector2,int[]"
		 */
		auto getMethodExact(const std::string& name, const std::string& signature) const ->std::shared_ptr<MapleMonoMethod>;

	
		auto getAllFields() const -> const std::vector<std::shared_ptr<MapleMonoField>>&;
		auto getAllProperties() const -> const std::vector<std::shared_ptr<MapleMonoProperty>>&;
		auto getAllMethods() const -> const std::vector<std::shared_ptr<MapleMonoMethod>>&;
		auto getAllAttributes() const ->std::vector<std::shared_ptr<MapleMonoClass>>;


		auto hasAttribute(std::shared_ptr<MapleMonoClass> monoClass) const-> bool;
		auto hasField(const std::string& name) const-> bool;

		auto isSubClassOf(const std::shared_ptr<MapleMonoClass> & monoClass) const-> bool;
		auto isInstanceOfType(MonoObject* object) const -> bool;
		
		auto getInstanceSize() const -> uint32_t;
		auto invokeMethod(const std::string& name, MonoObject* instance = nullptr, void** params = nullptr, uint32_t numParams = 0) ->MonoObject*;
		auto addInternalCall(const std::string& name, const void* method) -> void;
	
		auto createInstance(bool construct = true) ->std::shared_ptr<MapleMonoObject>;
		auto createInstance(void** params, uint32_t numParams)->std::shared_ptr<MapleMonoObject>;
		auto createInstance(const std::string& ctorSignature, void** params)->std::shared_ptr<MapleMonoObject>;

		static auto construct(MonoObject* object) -> void;

		inline auto getInternalClass() const { return clazz; }

		inline operator MonoClass* () { return clazz; }

	private:
		friend class MapleMonoAssembly;
		friend class MapleMonoObject;


		const MapleMonoAssembly* parentAssembly;
		MonoClass* clazz = nullptr;
		std::string nameSpace;
		std::string typeName;
		std::string fullName;

		mutable std::unordered_map<MethodId, std::shared_ptr<MapleMonoMethod>, MethodId::Hash, MethodId::Equals> methods;
		mutable std::unordered_map<std::string, std::shared_ptr<MapleMonoField>> fields;
		mutable std::unordered_map<std::string, std::shared_ptr<MapleMonoProperty>> properties;

		mutable bool hasCachedFields = false;
		mutable std::vector<std::shared_ptr<MapleMonoField>> cachedFieldList;

		mutable bool hasCachedProperties = false;
		mutable std::vector<std::shared_ptr<MapleMonoProperty>> cachedPropertyList;

		mutable bool hasCachedMethods = false;
		mutable std::vector<std::shared_ptr<MapleMonoMethod>> cachedMethodList;
	};
};