/////////////////////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "Frustum.h"


namespace Maple
{

	auto Frustum::from(const glm::mat4& projection) -> void
	{
		auto projInverse = glm::inverse(projection);
		glm::vec3 frustumCorners[8] = {
			{1.0f, 1.0f,    .0f},
			{1.0f, -1.0f,   .0f},
			{-1.0f, -1.0f,  .0f},
			{-1.0f, 1.0f,   .0f},
			{1.0f, 1.0f,  1.0f},
			{1.0f, -1.0f,   1.0f},
			{-1.0f, -1.0f,  1.0f},
			{-1.0f, 1.0f, 1.0f}
		};

		for (uint32_t j = 0; j < 8; j++)
		{
			auto invCorner = projInverse * glm::vec4(frustumCorners[j], 1.0f);
			vertices[j] = invCorner / invCorner.w;
		}
	}

};