/////////////////////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <glm/glm.hpp>
#include <cstdint>
#include "Engine/Core.h"
namespace Maple
{

	class BoundingBox;

	class MAPLE_EXPORT Ray
	{
	public:
		Ray() = default;
		Ray(const glm::vec3 & origin, const glm::vec3 & direction) 
		{
			set(origin, direction);
		}
		~Ray() {};
		inline auto operator==(const Ray & rhs) const { return origin == rhs.origin && direction == rhs.direction; }
		inline auto operator!=(const Ray & rhs) const { return origin != rhs.origin || direction != rhs.direction; }

		inline auto set(const glm::vec3 & origin, const glm::vec3 & direction) -> void
		{
			this->origin = origin;
			this->direction = glm::normalize(direction);
		}

		inline auto project(const glm::vec3 & point) const -> glm::vec3
		{
			glm::vec3 offset = point - origin;
			return origin + glm::dot(offset,direction)* direction;
		}

		inline auto distance(const glm::vec3 & point) const -> float
		{
			glm::vec3 projected = project(point);
			return glm::length(point - projected);
		}

		auto getClosestPoint(const Ray & ray) const ->glm::vec3;
		auto hit(const BoundingBox & box) const -> float;

		glm::vec3 origin;
		glm::vec3 direction;
	};
};