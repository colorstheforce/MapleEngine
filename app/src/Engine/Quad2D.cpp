//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "Quad2D.h"

namespace Maple
{
	

	auto Quad2D::getDefaultTexCoords() -> const std::array<glm::vec2, 4>&
	{
		static std::array<glm::vec2, 4> results =
		{
			glm::vec2{0, 1},
			glm::vec2{1, 1},
			glm::vec2{1, 0},
			glm::vec2{0, 0}
		};
		return results;
	}

	Quad2D::Quad2D()
	{
		texCoords = getDefaultTexCoords();
	}

	auto Quad2D::getTexCoords(const glm::vec2& min, const glm::vec2& max) -> const std::array<glm::vec2, 4>&
	{
		static std::array<glm::vec2, 4> results;
		{
			results[0] = { min.x, max.y };
			results[1] = max;
			results[2] = { max.x, min.y };
			results[3] = min;
		}
		return results;
	}
};