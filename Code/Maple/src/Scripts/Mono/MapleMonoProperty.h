//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine									//
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include "Mono.h"

namespace Maple
{
	class MapleMonoClass;

	class  MAPLE_EXPORT MapleMonoProperty
	{
	public:
		MapleMonoProperty(MonoProperty* monoProp);

		inline const auto& getName() const { return name; }
		auto get(MonoObject* instance) const ->MonoObject*;
		auto set(MonoObject* instance, void* value) const -> void;
		auto getIndexed(MonoObject* instance, uint32_t index) const ->MonoObject*;
		auto setIndexed(MonoObject* instance, uint32_t index, void* value) const-> void;
		auto isIndexed() const -> bool;
		auto getReturnType() const->std::shared_ptr<MapleMonoClass>;
		auto hasAttribute(MapleMonoClass* monoClass) -> bool;
		auto getAttribute(MapleMonoClass* monoClass) ->MonoObject*;
		auto getVisibility() ->MonoMemberVisibility;

	private:
		friend class MapleMonoClass;

		
	
		auto initializeDeferred() const -> void;

		std::string name;
		MonoProperty* monoProperty = nullptr;
		MonoMethod* getMethod = nullptr;
		MonoMethod* setMethod = nullptr;

		mutable std::shared_ptr<MapleMonoClass> returnType = nullptr;
		mutable bool indexed = false;
		mutable bool fullyInitialized = false;
	};
};