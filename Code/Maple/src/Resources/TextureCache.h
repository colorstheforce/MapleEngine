//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include <unordered_map>
#include "Resources.h"
#include "Engine/Core.h"
namespace Maple
{
	class Texture;

	class MAPLE_EXPORT TextureCache : public Resources<Texture> {};
};