//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Engine/Mesh.h"


namespace Maple 
{
	namespace MeshLoader
	{
		auto load(const std::string& obj, std::unordered_map<std::string, std::shared_ptr<Mesh>>&)-> void;
	};
};
