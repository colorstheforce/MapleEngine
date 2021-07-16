/////////////////////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include <glm/glm.hpp>
#include <cstdint>

namespace Maple
{
	class Rect2D final
	{
	public:
		Rect2D();
		Rect2D(int32_t left, int32_t top, int32_t right, int32_t bottom);
	private:
		glm::ivec2 leftTop;
		glm::ivec2 rightBottom;
	public:
        inline auto& getLeftTop() const { return leftTop; }
        inline auto setLeftTop(const glm::ivec2 & leftTop) { this->leftTop = leftTop; }

        inline auto& getRightBottom() const { return rightBottom; }
        inline auto setRightBottom(const glm::ivec2& rightBottom) { this->rightBottom = rightBottom; }

	};
};