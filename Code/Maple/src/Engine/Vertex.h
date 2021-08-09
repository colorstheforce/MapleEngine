
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <array>
#include <vulkan/vulkan.h>
namespace Maple
{
	struct Vertex 
	{
		glm::vec3 pos;
		glm::vec4 color;
		glm::vec2 texCoord;
		glm::vec3 normal;
		glm::vec3 tangent;
		auto operator==(const Vertex& other) const -> bool;
		auto operator-(const Vertex& right)->Vertex;
		auto operator+(const Vertex& right)->Vertex;
		auto operator*(float factor)->Vertex;
	};

	struct Vertex2D
	{
		glm::vec3 vertex;
		glm::vec3 uv;
		glm::vec4 color;
		auto operator==(const Vertex2D& other) const-> bool;
	};

};
namespace std {
	/*template<> struct hash<Maple::Vertex> {
		size_t operator()(const Maple::Vertex & vertex) const {
			return 
				(
				(hash<glm::vec3>()(vertex.pos) ^ 
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ 
				(hash<glm::vec2>()(vertex.texCoord) << 1
					);
		}
	};*/

	template<> struct hash<Maple::Vertex> {
		size_t operator()(const Maple::Vertex& vertex) const {
			return
					((hash<glm::vec3>()(vertex.pos) ^
					(hash<glm::vec2>()(vertex.texCoord) << 1) ^
					(hash<glm::vec4>()(vertex.color) << 1) ^
					(hash<glm::vec3>()(vertex.normal) << 1)
				));
		}
	};
}