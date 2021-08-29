//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include "Mono.h"
#include <vector>

namespace Maple::MonoHelper 
{
	auto executeCommand(const std::string & cmd)->std::string;
	auto compileScript(const std::string& script, const std::string& dllReference = "") -> bool;
	auto compileScript(const std::vector<std::string>& files, const std::string& dllReference = "")-> bool;
	auto getMethod(MonoImage* image, const std::string& method)->MonoMethod*;

	auto getClassName(MonoObject* obj, std::string& ns, std::string& typeName) -> void;
	auto getClassName(MonoClass* obj, std::string& ns, std::string& typeName) -> void;
	auto getClassName(MonoReflectionType* monoReflType, std::string& ns, std::string& typeName) -> void;

	auto throwIfException(MonoObject* exception) -> void;

	auto monoToString(MonoString* str)->std::string;
	auto stringToMono(const std::string& str)->MonoString*;


	auto getUINT16Class() -> MonoClass*;
	auto getINT16Class() -> MonoClass*;
	auto getUINT32Class() -> MonoClass*;
	auto getINT32Class() -> MonoClass*;
	auto getUINT64Class() -> MonoClass*;
	auto getINT64Class() -> MonoClass*;
	auto getStringClass() -> MonoClass*;
	auto getFloatClass() -> MonoClass*;
	auto getDoubleClass() -> MonoClass*;
	auto getBoolClass()->MonoClass*;
	auto getByteClass()->MonoClass*;
	auto getSByteClass() -> MonoClass*;
	auto getCharClass()  -> MonoClass*;
	auto getObjectClass() -> MonoClass*;

};