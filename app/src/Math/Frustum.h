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

	
	class Frustum
	{
	public:

		friend class DebugRenderer;
		friend class ShadowRenderer;

		static constexpr uint32_t FRUSTUM_VERTICES = 8;

		Frustum() noexcept = default;
		
		auto from(const glm::mat4 & projection) -> void;



	private:
		glm::vec3 vertices[FRUSTUM_VERTICES];
	};

};