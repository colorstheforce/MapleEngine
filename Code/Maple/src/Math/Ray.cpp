
#include "Ray.h"
#include "MathUtils.h"
#include "BoundingBox.h"

namespace Maple 
{
	auto Ray::getClosestPoint(const Ray& ray) const ->glm::vec3
	{
		glm::vec3 p13 = origin - ray.origin;
		glm::vec3 p43 = ray.direction;
		glm::vec3 p21 = direction;

		float d1343 = glm::dot(p13,p43);
		float d4321 = glm::dot(p43,p21);
		float d1321 = glm::dot(p13,p21);
		float d4343 = glm::dot(p43,p43);
		float d2121 = glm::dot(p21,p21);

		float d = d2121 * d4343 - d4321 * d4321;
		if (std::abs(d) < MathUtils::EPS)
			return origin;
		float n = d1343 * d4321 - d1321 * d4343;
		float a = n / d;

		return origin + a * direction;
	}


	auto Ray::hit(const BoundingBox& box) const -> float
	{
		if (!box.isDefined())
			return INFINITY;

		if (box.contains(origin))
			return 0.0f;

		float dist = INFINITY;

		if (origin.x < box.min.x && direction.x > 0.0f)
		{
			float x = (box.min.x - origin.x) / direction.x;
			if (x < dist)
			{
				auto point = origin + x * direction;
				if (point.y >= box.min.y && point.y <= box.max.y && point.z >= box.min.z && point.z <= box.max.z)
					dist = x;
			}
		}
		if (origin.x > box.max.x && direction.x < 0.0f)
		{
			float x = (box.max.x - origin.x) / direction.x;
			if (x < dist)
			{
				auto point = origin + x * direction;
				if (point.y >= box.min.y && point.y <= box.max.y && point.z >= box.min.z && point.z <= box.max.z)
					dist = x;
			}
		}

		if (origin.y < box.min.y && direction.y > 0.0f)
		{
			float x = (box.min.y - origin.y) / direction.y;
			if (x < dist)
			{
				auto point = origin + x * direction;
				if (point.x >= box.min.x && point.x <= box.max.x && point.z >= box.min.z && point.z <= box.max.z)
					dist = x;
			}
		}
		if (origin.y > box.max.y && direction.y < 0.0f)
		{
			float x = (box.max.y - origin.y) / direction.y;
			if (x < dist)
			{
				auto point = origin + x * direction;
				if (point.x >= box.min.x && point.x <= box.max.x && point.z >= box.min.z && point.z <= box.max.z)
					dist = x;
			}
		}


		if (origin.z < box.min.z && direction.z > 0.0f)
		{
			float x = (box.min.z - origin.z) / direction.z;
			if (x < dist)
			{
				auto point = origin + x * direction;
				if (point.x >= box.min.x && point.x <= box.max.x && point.y >= box.min.y && point.y <= box.max.y)
					dist = x;
			}
		}

		if (origin.z > box.max.z && direction.z < 0.0f)
		{
			float x = (box.max.z - origin.z) / direction.z;
			if (x < dist)
			{
				auto point = origin + x * direction;
				if (point.x >= box.min.x && point.x <= box.max.x && point.y >= box.min.y && point.y <= box.max.y)
					dist = x;
			}
		}

		return dist;
	}

};

