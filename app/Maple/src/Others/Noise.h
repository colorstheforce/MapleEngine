//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cstdint>

namespace Maple
{
	namespace Noise
	{
		auto perlinNoise(int32_t x, int32_t y) -> float;
	};
};