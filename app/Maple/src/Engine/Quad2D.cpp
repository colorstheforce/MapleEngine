//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "Quad2D.h"
#include "Engine/Interface/Texture.h"
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
		color = glm::vec4(1.f);
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

	auto Quad2D::setTexCoords(uint32_t x, uint32_t y, uint32_t w, uint32_t h) -> void
	{
		this->w = w;
		this->h = h;
		texCoords[0] = { x /		(float)texture->getWidth() ,(y + h) / (float)texture->getHeight() };
		texCoords[1] = { (x + w) /  (float)texture->getWidth() ,(y + h) / (float)texture->getHeight() };
		texCoords[2] = { (x + w) / (float)texture->getWidth() ,y / (float)texture->getHeight() };
		texCoords[3] = { x		/ (float)texture->getWidth() ,y / (float)texture->getHeight() };
	}

};