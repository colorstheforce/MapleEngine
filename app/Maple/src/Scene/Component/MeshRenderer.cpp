
#include "MeshRenderer.h"
#include "Application.h"
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"
#include "Scene/Entity/EntityManager.h"
#include "ImGui/ImGuiHelpers.h"
#include "Scene/Component/Transform.h"
#include "Engine/Mesh.h"

#include <imgui.h>


namespace Maple
{

	MeshRenderer::MeshRenderer(const std::shared_ptr<Mesh>& mesh)
		:mesh(mesh)
	{

	}

	auto MeshRenderer::loadFromModel() -> void
	{
		getMesh(meshName);
		if (mesh)
			mesh->setMaterial(material);
	}

	auto MeshRenderer::getMesh(const std::string& name) -> void
	{
		Entity ent{entity,app->getSceneManager()->getCurrentScene()};
		auto model = ent.tryGetComponent<Model>();
		if (model != nullptr)
		{	
			switch (model->type) {
				case  PrimitiveType::Cube:
					mesh = Mesh::createCube();
				break;
				case  PrimitiveType::Sphere:
					mesh = Mesh::createSphere();
					break;
				case PrimitiveType::File:
					mesh = model->resource->find(name);
				break;
			}
		}
		else 
		{
			model = ent.tryGetComponentFromParent<Model>();
			mesh = model->resource->find(name);
		}
	}

	Model::Model(const std::string& file)
		:filePath(file)
	{
		resource = MeshResource::get(file);
	}

};