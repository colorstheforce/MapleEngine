//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Timestep.h"
#include "Engine/Vertex.h"
#include "Engine/Vulkan/IndexBuffer.h"
#include "Engine/Vulkan/VertexBuffer.h"
#include "Engine/Interface/Texture.h"

namespace Maple 
{
	enum MeshType
	{
		MESH,
		TERRAIN,
	};

	class DescriptorSet;
	class Camera;


	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(const std::shared_ptr<VertexBuffer> & vertexBuffer,
			const std::shared_ptr<IndexBuffer> & indexBuffer);
		inline auto setMaterial(const std::shared_ptr<Material>& material) {
			this->material = material;
		}
		inline auto setIndicesSize(uint32_t size) { this->size = size; }
		inline auto getSize() { return size; }
		inline auto& getIndexBuffer() { return indexBuffer; }
		inline auto& getVertexBuffer() { return vertexBuffer; }
		inline auto& getMaterial() { return material; }
		inline auto& getDescriptorSet() { return descriptorSet; }
		inline void setDescriptorSet(const std::shared_ptr<DescriptorSet> & set) { descriptorSet = set; }
		inline auto& isActive() const { return active; }
		inline auto setActive(bool active) { this->active = active; }

		inline auto& getName() const { return name; }
		inline auto setName(const std::string & name) { this->name = name; }

		virtual auto getType() -> MeshType { return MESH; }

		static auto createQuad()->std::shared_ptr< Mesh>;
		static auto createCube()->std::shared_ptr< Mesh>;
		static auto createPlane(float w,float h,const glm::vec3 & normal)->std::shared_ptr< Mesh>;


		static auto generateNormals(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) -> void;
		static auto generateTangents(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) -> void;

	protected:

		static auto generateTangent(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec2& ta, const glm::vec2& tb, const glm::vec2& tc) -> glm::vec3;

		std::shared_ptr<IndexBuffer> indexBuffer;
		std::shared_ptr<VertexBuffer> vertexBuffer;
		std::shared_ptr<Texture> texture;
		std::shared_ptr<DescriptorSet> descriptorSet;
		std::shared_ptr<Material> material;

		uint32_t size = 0;
		bool active = true;
		std::string name;


		static std::shared_ptr<Mesh> defaultCube;


	};
};