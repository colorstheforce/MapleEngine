//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine									//
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Mono.h"
#include <string>
#include <vector>

namespace Maple
{
	class MapleMonoMethod 
	{
	
	public:
		~MapleMonoMethod();
		MapleMonoMethod(MonoMethod* method);
		auto invoke(MonoObject* instance, void** params) ->MonoObject*;
		auto invokeVirtual(MonoObject* instance, void** params)->MonoObject*;

		/**
		 * Gets a thunk for this method. A thunk is a C++ like function pointer that you can use for calling the method.
		 * @note	This is the fastest way of calling managed code.
		 */
		auto getThunk() const -> void*;
		auto getName() const ->std::string;
		auto getReturnType() const->std::shared_ptr<MapleMonoClass>;
		auto getNumParameters() const -> uint32_t;
		auto getParameterType(uint32_t paramIdx) const->std::shared_ptr<MapleMonoClass>;
		auto isStatic() const -> bool;
		auto hasAttribute(std::shared_ptr<MapleMonoClass> monoClass) const -> bool;
		auto getAttribute(std::shared_ptr<MapleMonoClass> monoClass) const ->MonoObject*;
		auto getVisibility() ->MonoMemberVisibility;

	private:
		friend class MapleMonoClass;
		friend class MapleMonoProperty;

	

		auto cacheSignature() const -> void;

		MonoMethod* method = nullptr;
		mutable std::shared_ptr<MapleMonoClass> cachedReturnType = nullptr;

		mutable std::vector< std::shared_ptr<MapleMonoClass>> cachedParameters;

		mutable uint32_t cachedNumParameters = 0;
		mutable bool _static = false;
		mutable bool hasCachedSignature = false;
	};
};