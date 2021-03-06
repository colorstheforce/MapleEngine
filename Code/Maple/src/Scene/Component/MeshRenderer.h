//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Game Engine			                    //
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <vector>
#include "Component.h"
#include "Resources/MeshResource.h"
#include <cereal/types/memory.hpp>
#include "Engine/Material.h"


namespace Maple 
{
	class Mesh;
	class MeshResource;

	enum class PrimitiveType : int32_t
	{
		Plane = 0,
		Quad = 1,
		Cube = 2,
		Pyramid = 3,
		Sphere = 4,
		Capsule = 5,
		Cylinder = 6,
		Terrain = 7,
		File = 8
	};


	class MAPLE_EXPORT Model final : public Component
	{
	public:
		Model() = default;
		Model(const std::string& file);

		template<class Archive>
		auto save(Archive& archive) const -> void
		{
			archive(filePath, type, entity);
		}

		template<class Archive>
		auto load(Archive& archive) -> void
		{
			archive(filePath, type, entity);
			if(type == PrimitiveType::File)
				resource = MeshResource::get(filePath);
		}

		std::string filePath;
		PrimitiveType type;
		std::shared_ptr<MeshResource> resource;
	};

	class MAPLE_EXPORT MeshRenderer : public Component
	{
	public:

		MeshRenderer() = default;
		MeshRenderer(const std::shared_ptr<Mesh> & mesh);

		template<class Archive>
		auto save(Archive& archive) const -> void
		{
			archive(mesh->getName(), entity, cereal::make_nvp("material", mesh->getMaterial()));
		}

		template<class Archive>
		auto load(Archive& archive) -> void
		{
			archive(meshName, entity, cereal::make_nvp("material", material));
		}

		auto loadFromModel() -> void;

		inline auto getMesh() { if (mesh == nullptr) getMesh(meshName);  return mesh; }

	private:
		std::shared_ptr<Mesh> mesh;
		auto getMesh(const std::string& name) -> void;
		std::string meshName;
		std::shared_ptr<Material> material;

	};

};
