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
	auto compileScript(MonoDomain* domain, const std::vector<std::string> & files, const std::string& dllReference = "")->MonoAssembly*;
	auto getMethod(MonoImage* image, const std::string& method)->MonoMethod*;
};