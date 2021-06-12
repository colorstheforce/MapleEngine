//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 
////////////////////////////////////////////////////////////////////////////// 

#pragma once

#include <cstddef>

namespace Maple
{
	namespace HashCode
	{
		inline void hashCode(std::size_t& seed) { }

		template <typename T, typename... Rest>
		inline void hashCode(std::size_t& seed, const T& v, Rest... rest)
		{
			std::hash<T> hasher;
			seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			hashCode(seed, rest...);
		}
	};
};
	