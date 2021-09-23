//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Game Engine			                    //

//////////////////////////////////////////////////////////////////////////////

#include "Scene.h"
#include "Entity/Entity.h"
#include "Entity/EntityManager.h"
#include "SceneGraph.h"
#include "Scene/Component/Transform.h"
#include "Scene/Component/Light.h"
#include "Scene/Component/CameraControllerComponent.h"
#include "Scene/Component/MeshRenderer.h"
#include "Scene/Component/Sprite.h"

#include "Engine/Material.h"
#include "Engine/CameraController.h"
#include "Engine/Profiler.h"
#include "Others/StringUtils.h"
#include "Engine/Camera.h"
#include "Devices/Input.h"
#include "Others/Serialization.h"

#include "Scripts/Mono/MonoSystem.h"
#include "Others/Console.h"
#include <fstream>
#include <filesystem>

#include "Application.h"





namespace Maple { 

	Scene::Scene(const std::string& initName)
		:name(initName)
	{
		LOGV("{0} {1}", __FUNCTION__,initName);
		entityManager = std::make_shared<EntityManager>(this);
		entityManager->addDependency<Camera, Transform>();
		entityManager->addDependency<Light, Transform>();
		entityManager->addDependency<MeshRenderer, Transform>();
		entityManager->addDependency<Model, Transform>();
		entityManager->addDependency<Sprite, Transform>();
		entityManager->addDependency<AnimatedSprite, Transform>();

		sceneGraph = std::make_shared<SceneGraph>();
		sceneGraph->init(entityManager->getRegistry());
	}

	entt::registry& Scene::getRegistry()
	{
		return entityManager->getRegistry();
	}

	auto Scene::setSize(uint32_t w, uint32_t h) -> void
	{
		width = w;
		height = h;
	}

	auto Scene::saveTo(const std::string& path, bool binary) -> void
	{
		PROFILE_FUNCTION();
		if (dirty) {
			LOGV("save to disk");
			if (path != "" && path != filePath)
			{
				filePath = path + StringUtils::delimiter + name + ".scene";
			}
			if (filePath == "")
			{
				filePath = name + ".scene";
			}
			Serialization::serialize(this);
			dirty = false;
		}
	}

	auto Scene::loadFrom()-> void
	{
		PROFILE_FUNCTION();
		if(filePath != ""){
			entityManager->clear();
			sceneGraph->disconnectOnConstruct(true, getRegistry());
			Serialization::loadScene(this, filePath);
			sceneGraph->disconnectOnConstruct(false, getRegistry());
		}
	}

	auto Scene::createEntity() -> Entity
	{
		dirty = true;
		return entityManager->create();
	}

	auto Scene::createEntity(const std::string& name) -> Entity
	{
		PROFILE_FUNCTION();
		dirty = true;
		int32_t i = 0; 
		auto entity = entityManager->getEntityByName(name);
		while (entity.valid()) {
			entity = entityManager->getEntityByName(name+"("+std::to_string(i + 1)+")");
			i++;
		}
		return entityManager->create(i == 0 ? name : name + "(" + std::to_string(i) + ")");
	}

	auto Scene::duplicateEntity(const Entity& entity, const Entity& parent) -> void
	{
		PROFILE_FUNCTION();
		dirty = true;

		Entity newEntity = entityManager->create();
		
		if (parent)
			newEntity.setParent(parent);

		copyComponents(entity, newEntity);
	}

	auto Scene::duplicateEntity(const Entity& entity)  -> void
	{
		dirty = true;
		Entity newEntity = entityManager->create();
		//COPY
		copyComponents(entity,newEntity);
	}

	auto Scene::getCamera() ->std::pair<Camera*, Transform*>
	{
		auto camsEttView = entityManager->getEntitiesWithType<Camera>();
		if (!camsEttView.empty() && gameView)
		{
			Camera& sceneCam = camsEttView.front().getComponent<Camera>();
			Transform& sceneCamTr = camsEttView.front().getComponent<Transform>();
			return { &sceneCam,&sceneCamTr };
		}
		return { overrideCamera,overrideTransform };
	}


	auto Scene::copyComponents(const Entity& from, const Entity& to) -> void
	{
		LOGW("Not implementation {0}", __FUNCTION__);
	}

	auto Scene::onInit() -> void
	{
		if (initCallback != nullptr) 
		{
			initCallback(this);
		}
		Application::get()->getSystemManager()->getSystem<MonoSystem>()->onStart(this);
	}

	auto Scene::onClean() -> void
	{

	}

	auto Scene::updateCameraController(float dt) -> void
	{
		PROFILE_FUNCTION();
		auto controller = entityManager->getRegistry().group<CameraControllerComponent>(entt::get<Transform>);
		for (auto entity : controller)
		{
			const auto mousePos = Input::getInput()->getMousePosition();
			auto& [con, trans] = controller.get<CameraControllerComponent, Transform>(entity);
			if (Application::get()->isSceneActive() && 
				Application::get()->getEditorState() == EditorState::Play&&
				con.getController() )
			{
				con.getController()->handleMouse(trans, dt, mousePos.x, mousePos.y);
				con.getController()->handleKeyboard(trans, dt);
			}
		}
	}

	auto Scene::onUpdate(float dt) -> void
	{
		PROFILE_FUNCTION();
		updateCameraController(dt);
		sceneGraph->update(entityManager->getRegistry());
		auto view = entityManager->getRegistry().group<AnimatedSprite>(entt::get<Transform>);
		for (auto entity : view)
		{
			const auto& [anim, trans] = view.get<AnimatedSprite, Transform>(entity);
			anim.onUpdate(dt);
		}
	}

};
