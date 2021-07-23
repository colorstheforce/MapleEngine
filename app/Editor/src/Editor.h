//////////////////////////////////////////////////////////////////////////////
// This file is part of the Vulkan Assignment								// 
// Copyright ?2020-2022 Prime Zeng                                          // 
////////////////////////////////////////////////////////////////////////////// 

#pragma once
#include <memory>
#include "Application.h"
#include "EditorWindow.h"
#include "Scene/Component/Transform.h"
#include "Engine/CameraController.h"
#include "FileSystem/File.h"
#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace Maple
{
	class EditorPlugin;
	class Texture2D;
	class SceneWindow;
	class TextureAtlas;
	class RenderManager;
	class Quad2D;
	class Ray;

	class Editor final : public Application 
	{
	public:
		Editor(AppDelegate* appDelegate);
		auto init() -> void override;
		auto onImGui() -> void override;
		auto onUpdate(const Timestep& delta) -> void override;


		auto onRenderDebug() -> void override;

		auto setSelected(const entt::entity& selectedNode) -> void;
		auto setCopiedEntity(const entt::entity& selectedNode, bool cut = false) -> void;

		auto getScreenRay(int32_t x, int32_t y, Camera* camera, int32_t width, int32_t height)->Ray;


		template<class T>
		inline auto getWindow() {
			return std::static_pointer_cast<T>(editorWindows[typeid(T).hash_code()]);
		}

		template<class T>
		auto addSubWindow() -> void;

		inline auto& getSelected() const { return selectedNode; }
		inline auto& getPrevSelected() const { return prevSelectedNode; }
		inline auto& getCopiedEntity() const { return copiedNode; }
		inline auto  isCutCopyEntity() const { return cutCopyEntity; }
		inline auto& getComponentIconMap() const { return iconMap; }
		inline auto& getComponentIconMap() { return iconMap; }



		inline auto getImGuizmoOperation() const { return imGuizmoOperation; }
		inline auto setImGuizmoOperation(uint32_t imGuizmoOperation) { this->imGuizmoOperation = imGuizmoOperation; }
		inline auto& getCamera() { return camera; }
		inline auto& getEditorCameraTransform() { return editorCameraTransform; }
		inline auto& getEditorCameraController() { return editorCameraController; }
		inline auto getTextureAtlas() { return textureAtlas; }

		auto onImGuizmo() -> void;

		auto onSceneCreated(Scene* scene) -> void override;
	

		auto getIconFontIcon(const std::string& file) -> const char*;

		auto openFile(const std::string& file) -> void;

		auto drawGrid() -> void;
		
		auto getIcon(FileType type)->Quad2D*;
		auto processIcons() -> void;

		auto addPlugin(EditorPlugin* plugin) -> void;
		auto addFunctionalPlugin(const std::function<void(Editor*)> & callback) -> void;

		auto clickObject(const Ray& ray) -> void;
		auto focusCamera(const glm::vec3& point, float distance, float speed = 1.0f) -> void;
	private:
		auto drawPlayButtons() -> void;
		auto drawMenu() -> void;
		auto beginDockSpace()  -> void;
		auto endDockSpace()  -> void;
		auto loadCachedScene()-> void;
		auto cacheScene()-> void;
		std::unordered_map<size_t, std::shared_ptr<EditorWindow>> editorWindows;

		entt::entity selectedNode = entt::null;
		entt::entity prevSelectedNode = entt::null;
		entt::entity copiedNode = entt::null;
		bool cutCopyEntity = false;
		std::unordered_map<size_t, const char*> iconMap;

		uint32_t imGuizmoOperation = 4;
		bool showGizmos = true;

		std::unique_ptr<Camera> camera;
		Transform editorCameraTransform;
		EditorCameraController editorCameraController;
		bool cameraSelected = false;

		
		//need to be optimized. should use Atlats to cache.
		std::unordered_map<FileType, std::string> cacheIcons;
		std::shared_ptr<TextureAtlas> textureAtlas;
		std::vector<std::unique_ptr<EditorPlugin>> plugins;


	private:
		bool transitioningCamera = false;
		glm::vec3 cameraDestination;
		glm::vec3 cameraStartPosition;
		float cameraTransitionStartTime = 0.0f;
		float cameraTransitionSpeed = 0.0f;

	};

	template<class T>
	auto Editor::addSubWindow() -> void
	{
		editorWindows.emplace(typeid(T).hash_code(), std::make_shared<T>());
	}
};