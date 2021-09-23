//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once

// Mono
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/loader.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/exception.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/tokentype.h>
#include <mono/metadata/attrdefs.h>
#include <mono/utils/mono-logger.h>
#include <memory>
#include "Engine/Core.h"

enum class MonoPrimitiveType
{
	Boolean,
	Char,
	I8,
	U8,
	I16,
	U16,
	I32,
	U32,
	I64,
	U64,
	R32,
	R64,
	String,
	ValueType,
	Class,
	Array,
	Generic,
	Unknown
};

enum class MonoMemberVisibility 
{
	Private,
	Protected,
	Internal,
	ProtectedInternal,
	Public
};

namespace Maple {
	class MapleMonoMethod;
	class MapleMonoField;
	class MapleMonoProperty;
	class MapleMonoClass;
	class MapleMonoAssembly;
	class MapleMonoObject;
	struct MonoScriptInstance;
};
