/////////////////////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include <glm/glm.hpp>
#include <algorithm>

namespace Maple
{
	namespace MathUtils 
	{
		constexpr float EPS = 0.000001f;

		inline float lerp(float from, float to, float t, bool clamp01 = true)
		{
			if (clamp01)
			{
				t = std::min<float>(std::max<float>(t, 0), 1);
			}
			return from + (to - from) * t;
		}

		inline auto lerp(const glm::vec3 & from,const glm::vec3& to, float t) { return from * (1.0f - t) + to * t; }


		// vec4 comparators
		inline bool operator >=(const glm::vec4& left, const glm::vec4& other)
		{
			return left.x >= other.x && left.y >= other.y && left.z >= other.z && left.w >= other.w;
		}

		inline bool operator <=(const glm::vec4& left, const glm::vec4& other)
		{
			return left.x <= other.x && left.y <= other.y && left.z <= other.z && left.w <= other.w;;
		}


		// vec3 comparators
		inline bool operator >=(const glm::vec3& left, const glm::vec3& other)
		{
			return left.x >= other.x && left.y >= other.y && left.z >= other.z;
		}

		inline bool operator <=(const glm::vec3& left, const glm::vec3& other)
		{
			return left.x <= other.x && left.y <= other.y && left.z <= other.z;
		}


		// vec3 comparators
		inline bool operator >=(const glm::vec2& left, const glm::vec2& other)
		{
			return left.x >= other.x && left.y >= other.y ;
		}

		inline bool operator <=(const glm::vec2& left, const glm::vec2& other)
		{
			return left.x <= other.x && left.y <= other.y ;
		}

		//only support for float value and vector
		template <class T>
		typename std::enable_if<
			std::is_floating_point<T>::value || 
			std::is_same<T,glm::vec2>::value || 
			std::is_same<T, glm::vec3>::value || 
			std::is_same<T, glm::vec4>::value,bool>::type equals(const T& lhs, const T& rhs) {
			T eps(0.000001f); 
			return lhs + eps >= rhs && lhs - eps <= rhs; 
		}
	};
};
