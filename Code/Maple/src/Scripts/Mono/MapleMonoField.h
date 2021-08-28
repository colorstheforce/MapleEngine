//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine									//
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include "Mono.h"

namespace Maple
{
	class MapleMonoField
	{
	public:
		friend class MapleMonoClass;
		inline auto& getName() const { return name; }
		auto getType()->std::shared_ptr<MapleMonoClass>;
		auto get(MonoObject* instance, void* outValue) -> void;
		auto set(MonoObject* instance, void* value) -> void;
		auto hasAttribute(std::shared_ptr<MapleMonoClass> monoClass) -> bool;
		auto getAttribute(std::shared_ptr<MapleMonoClass> monoClass) ->MonoObject*;
		auto getVisibility() -> MonoMemberVisibility;
		auto isStatic() -> bool;
		/**
		 * Retrieves value currently set in the field on the specified object instance. If field is static object instance
		 * can be null. If returned value is a value type it will be boxed.
		 */
		auto getBoxed(MonoObject* instance) -> MonoObject *;
		MapleMonoField(MonoClassField* field);
	private:
		std::string name;
		MonoClassField* monoField = nullptr;
		std::shared_ptr<MapleMonoClass> fieldType;
	};
};