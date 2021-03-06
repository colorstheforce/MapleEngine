//////////////////////////////////////////////////////////////////////////////
// This file is part of the Vulkan Assignment								// 
// Copyright ?2020-2022 Prime Zeng                                          // 
////////////////////////////////////////////////////////////////////////////// 

#include "Main.h"
#include "Editor.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

#include "SceneWindow.h"
#include "HierarchyWindow.h"
#include "PropertiesWindow.h"
#include "AssetsWindow.h"
#include "DisplayZeroWindow.h"

#include "Engine/TextureAtlas.h"
#include "Engine/Camera.h"
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"
#include "Scene/Component/Component.h"
#include "Scene/Component/Transform.h"
#include "Scene/Component/Light.h"
#include "Scene/Component/Sprite.h"
#include "Scene/Entity/Entity.h"
#include "Devices/Input.h"
#include "ImGui/ImGuiHelpers.h"
#include "IconsMaterialDesignIcons.h"

#include "FileSystem/File.h"
#include "FileSystem/MeshLoader.h"
#include "Others/StringUtils.h"
#include "Scene/Component/MeshRenderer.h"

#include "Engine/Renderer/RenderManager.h"
#include "Engine/Renderer/ForwardRenderer.h"
#include "Engine/Renderer/DeferredRenderer.h"
#include "Engine/Renderer/SkyboxRenderer.h"
#include "Engine/Renderer/ShadowRenderer.h"
#include "Engine/Renderer/GridRenderer.h"
#include "Engine/Renderer/PreProcessRenderer.h"
#include "Engine/Renderer/OmniShadowRenderer.h"
#include "Engine/Renderer/Renderer2D.h"

#include "Scripts/Mono/MonoVirtualMachine.h"

#include "EditorPlugin.h"
#include "Plugin/PluginWindow.h"

#include "Math/BoundingBox.h"
#include "Math/Ray.h"
#include "Math/MathUtils.h"
#include "Scripts/Mono/MonoComponent.h"


namespace Maple 
{

	Editor::Editor(AppDelegate* appDelegate) : Application(appDelegate)
	{

	}
#define addWindow(T) editorWindows.emplace(typeid(T).hash_code(),std::make_shared<T>())
	auto Editor::init() -> void
	{
		Application::init();
		auto renderManager = std::make_unique<RenderManager>();
		renderManager->setEditor(true);
		//off-screen
	

		renderManager->addRender(std::make_unique<ShadowRenderer>());
		renderManager->addRender(std::make_unique<DeferredRenderer>(window->getWidth(), window->getHeight()));
		renderManager->addRender(std::make_unique<SkyboxRenderer>(window->getWidth(), window->getHeight()));
		renderManager->addRender(std::make_unique<GridRenderer>(window->getWidth(), window->getHeight()));
		renderManager->addRender(std::make_unique<Renderer2D>(window->getWidth(), window->getHeight()));
		renderManager->init(window->getWidth(), window->getHeight());

		renderManagers.emplace_back(std::move(renderManager));

		addWindow(SceneWindow);
		addWindow(DisplayZeroWindow);
		addWindow(HierarchyWindow);
		addWindow(PropertiesWindow);
		addWindow(AssetsWindow); 
	

	

		iconMap[typeid(Transform).hash_code()] = ICON_MDI_VECTOR_LINE;
		iconMap[typeid(Editor).hash_code()] = ICON_MDI_SQUARE;
		iconMap[typeid(Light).hash_code()] = ICON_MDI_LIGHTBULB;
		iconMap[typeid(Camera).hash_code()] = ICON_MDI_CAMERA;
		iconMap[typeid(MeshRenderer).hash_code()] = ICON_MDI_SHAPE;
		iconMap[typeid(Environment).hash_code()] = ICON_MDI_EARTH;
		iconMap[typeid(Sprite).hash_code()] = ICON_MDI_IMAGE;
		iconMap[typeid(MonoComponent).hash_code()] = ICON_MDI_LANGUAGE_CSHARP;


		ImGuizmo::SetGizmoSizeClipSpace(0.25f);
		auto winSize = window->getWidth() / (float) window->getHeight();

		camera = std::make_unique<Camera>(
			60, 0.1, 32000, winSize);
		editorCameraController.setCamera(camera.get());

		setEditorState(EditorState::Preview);

		if (File::fileExists("default.scene")) {
			sceneManager->addSceneFromFile("default.scene");
			sceneManager->switchScene("default.scene");
		}

		processIcons();

		MonoVirtualMachine::get()->compileAssembly([&](void*) {
			MonoVirtualMachine::get()->loadAssembly("./", "MapleLibrary.dll");
			MonoVirtualMachine::get()->loadAssembly("./", "MapleAssembly.dll");
			addWindow(PluginWindow);
		});
	}

	auto Editor::onImGui() -> void
	{
		drawMenu();
		dialog.onImGui();
		beginDockSpace();
		for (auto & win : editorWindows)
		{
			win.second->onImGui();
		}
		endDockSpace();
		Application::onImGui();
	}

	auto Editor::onUpdate(const Timestep& delta) -> void
	{
		Application::onUpdate(delta);

		auto currentScene = sceneManager->getCurrentScene();
		if (getEditorState() == EditorState::Preview)
		{
			auto& registry = currentScene->getRegistry();

			if (isSceneActive())
			{
				const auto mousePos = Input::getInput()->getMousePosition();
				editorCameraController.handleMouse(editorCameraTransform, delta, mousePos.x, mousePos.y);
				editorCameraController.handleKeyboard(editorCameraTransform, delta);
			}

			if (!Input::getInput()->isMouseHeld(KeyCode::MouseKey::ButtonRight)
				&& !ImGuizmo::IsUsing() && isSceneActive()
				&& selectedNode != entt::null)
			{
				if (Input::getInput()->isKeyPressed(KeyCode::Id::Q))
				{
					setImGuizmoOperation(4);
				}

				if (Input::getInput()->isKeyPressed(KeyCode::Id::W))
				{
					setImGuizmoOperation(ImGuizmo::OPERATION::TRANSLATE);
				}

				if (Input::getInput()->isKeyPressed(KeyCode::Id::E))
				{
					setImGuizmoOperation(ImGuizmo::OPERATION::ROTATE);
				}

				if (Input::getInput()->isKeyPressed(KeyCode::Id::R))
				{
					setImGuizmoOperation(ImGuizmo::OPERATION::SCALE);
				}

				if (Input::getInput()->isKeyPressed(KeyCode::Id::T))
				{
					setImGuizmoOperation(ImGuizmo::OPERATION::BOUNDS);
				}
			}

			if ((Input::getInput()->isKeyHeld(KeyCode::Id::LeftControl)
				||
				Input::getInput()->isKeyHeld(KeyCode::Id::RightControl)) && Input::getInput()->isKeyPressed(KeyCode::Id::S)
				) {

				serialize();

			}
			editorCameraTransform.setWorldMatrix(glm::mat4(1.f));
		}



/*
		if (transitioningCamera)
		{
			if (cameraTransitionStartTime < 0.0f)
				cameraTransitionStartTime = delta.getMilliseconds();

			float focusProgress = std::min((delta.getMilliseconds() - cameraTransitionStartTime) / cameraTransitionSpeed, 1.f);
			auto newCameraPosition = MathUtils::lerp(cameraStartPosition,cameraDestination, focusProgress);
			editorCameraTransform.setLocalPosition(newCameraPosition);

			if (MathUtils::equals(editorCameraTransform.getLocalPosition(),cameraDestination))
				transitioningCamera = false;
		}
*/

		for (auto& plugin : plugins)
		{
			if (!plugin->isInited())
			{
				plugin->process(this);
				plugin->setInited(true);
			}
		}
	}



	auto Editor::onRenderDebug() -> void
	{
		if (selectedNode == entt::null) {
			return;
		}

		auto& registry = getSceneManager()->getCurrentScene()->getRegistry();
		if (cameraSelected || camera->isOrthographic()) 
		{
			auto view = registry.group<Camera>(entt::get<Transform>);

			for (auto v : view)
			{
				auto & [camera, trans] = registry.get<Camera, Transform>(v);
				debugRender.drawFrustum(camera.getFrustum(glm::inverse(trans.getWorldMatrix())));
			}
		}

		if (auto sprite = registry.try_get<Sprite>(selectedNode)) {
			auto& transform = registry.get<Transform>(selectedNode);
			auto pos = transform.getWorldPosition() + glm::vec3(sprite->getQuad().getOffset(), 0);
			auto w = sprite->getQuad().getWidth();
			auto h = sprite->getQuad().getHeight();
			debugRender.drawRect(pos.x, pos.y, w, h);
		}

		if (auto sprite = registry.try_get<AnimatedSprite>(selectedNode)) {
			auto& transform = registry.get<Transform>(selectedNode);
			auto pos = transform.getWorldPosition() + glm::vec3(sprite->getQuad().getOffset(), 0);
			auto w = sprite->getQuad().getWidth();
			auto h = sprite->getQuad().getHeight();
			debugRender.drawRect(pos.x, pos.y, w, h);
		}

		if (auto meshRender = registry.try_get<MeshRenderer>(selectedNode)) {
			auto& transform = registry.get<Transform>(selectedNode);
			if (auto mesh = meshRender->getMesh(); mesh != nullptr) {
				auto& worldTransform = transform.getWorldMatrix();
				auto bbCopy = mesh->getBoundingBox()->transform(worldTransform);
				debugRender.drawBox(bbCopy, { 1,1,1,1 });
			}
		}


		//drawGrid();


	}

	auto Editor::setSelected(const entt::entity& node) -> void
	{
		prevSelectedNode = selectedNode;
		selectedNode = node;
		
		cameraSelected = node != entt::null && getSceneManager()->getCurrentScene()->getRegistry().try_get<Camera>(selectedNode) != nullptr;
	}

	auto Editor::setCopiedEntity(const entt::entity& selectedNode, bool cut) -> void
	{
		copiedNode = selectedNode;
	}

	auto Editor::onImGuizmo() -> void
	{
		auto view = glm::inverse(editorCameraTransform.getWorldMatrix());
		auto proj = camera->getProjectionMatrix();

		if (selectedNode == entt::null || imGuizmoOperation == ImGuizmo::SELECT)
			return;

		if (showGizmos)
		{
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetOrthographic(camera->isOrthographic());

			auto& registry = sceneManager->getCurrentScene()->getRegistry();
			auto transform = registry.try_get<Transform>(selectedNode);
			if (transform != nullptr)
			{
				auto model = transform->getWorldMatrix();

				float delta[16];

				ImGuizmo::Manipulate(
					glm::value_ptr(view),
					glm::value_ptr(proj),
					static_cast<ImGuizmo::OPERATION>(imGuizmoOperation),
					ImGuizmo::LOCAL,
					glm::value_ptr(model),
					delta,
					nullptr);

				if (ImGuizmo::IsUsing())
				{
					if (static_cast<ImGuizmo::OPERATION>(imGuizmoOperation) == ImGuizmo::OPERATION::SCALE)
					{
						auto mat = glm::make_mat4(delta);

						transform->setLocalScale(transform->getLocalScale() * Transform::getScaleFromMatrix(mat));
					}
					else
					{
						auto mat = glm::make_mat4(delta) * transform->getLocalMatrix();
						transform->setLocalTransform(mat);
						//TOOD
					}
				}
			}
		}
	}

	auto Editor::drawPlayButtons() -> void
	{
		auto x = (ImGui::GetWindowContentRegionMax().x / 2.0f) - (1.5f * (ImGui::GetFontSize() + ImGui::GetStyle().ItemSpacing.x));
		
		ImGui::SameLine(x);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.2f, 0.7f, 0.0f));

		if (getEditorState() == EditorState::Next)
			setEditorState(EditorState::Paused);

		bool selected;
		{
			selected = getEditorState() == EditorState::Play;
			if (selected)
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.28f, 0.56f, 0.9f, 1.0f));

			if (ImGui::Button(selected ? ICON_MDI_STOP : ICON_MDI_PLAY))
			{
				setEditorState(selected ? EditorState::Preview : EditorState::Play);

				selectedNode = entt::null;
				if (selected)
					loadCachedScene();
				else
				{
					cacheScene();
					sceneManager->getCurrentScene()->onInit();
				}
			}

			ImGuiHelper::tooltip("Play");
			if (selected)
				ImGui::PopStyleColor();
		}

		//ImGui::SameLine();

		{
			selected = getEditorState() == EditorState::Paused;
			if (selected)
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.28f, 0.56f, 0.9f, 1.0f));

			if (ImGui::Button(ICON_MDI_PAUSE))
				setEditorState(selected ? EditorState::Play : EditorState::Paused);

			ImGuiHelper::tooltip("Pause");


			if (selected)
				ImGui::PopStyleColor();
		}


		{
			selected = getEditorState() == EditorState::Next;
			if (selected)
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.28f, 0.56f, 0.9f, 1.0f));

			if (ImGui::Button(ICON_MDI_STEP_FORWARD))
				setEditorState(EditorState::Next);


			ImGuiHelper::tooltip("Next");

			if (selected)
				ImGui::PopStyleColor();
		}

		ImGui::PopStyleColor();

	}

	auto Editor::drawMenu() -> void
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit"))
				{
				}

				if (ImGui::MenuItem("Open File"))
				{

				}

				if (ImGui::MenuItem("Save (Ctrl + S)")) {
					serialize();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Scripts")) 
			{
				if (ImGui::MenuItem("Compile C# Assembly"))
				{
					dialog.show("Compiling ....");
					MonoVirtualMachine::get()->compileAssembly([&](void*) {
						dialog.close();
						MonoVirtualMachine::get()->loadAssembly("./", "MapleAssembly.dll");
					});
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("GBuffer"))
			{
				auto event = std::make_unique<DeferredTypeEvent>();

				if (ImGui::MenuItem("Depth"))
				{
					event->setDeferredType(0);
					Application::get()->getEventDispatcher().postEvent(std::move(event));
				}

				if (ImGui::MenuItem("Color"))
				{
					event->setDeferredType(1);
					Application::get()->getEventDispatcher().postEvent(std::move(event));
				}

				if (ImGui::MenuItem("Position"))
				{
					event->setDeferredType(2);
					Application::get()->getEventDispatcher().postEvent(std::move(event));
				}

				if (ImGui::MenuItem("Normal"))
				{
					event->setDeferredType(3);
					Application::get()->getEventDispatcher().postEvent(std::move(event));
				}

				if (ImGui::MenuItem("ShadowMap"))
				{
					event->setDeferredType(4);
					Application::get()->getEventDispatcher().postEvent(std::move(event));
				}

				if (ImGui::MenuItem("ShadowCubeMap"))
				{
					event->setDeferredType(5);
					Application::get()->getEventDispatcher().postEvent(std::move(event));
				}

				if (ImGui::MenuItem("Deferred"))
				{
					event->setDeferredType(6);
					Application::get()->getEventDispatcher().postEvent(std::move(event));
				}


				ImGui::EndMenu();
			}


			drawPlayButtons();

			ImGui::EndMainMenuBar();
		}
	}

	auto Editor::beginDockSpace() -> void
	{
		static bool p_open = true;
		static bool opt_fullscreen_persistant = true;
		static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton;
		bool opt_fullscreen = opt_fullscreen_persistant;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();

			auto pos = viewport->Pos;
			auto size = viewport->Size;
			bool menuBar = true;
			if (menuBar)
			{
				const float infoBarSize = ImGui::GetFrameHeight();
				pos.y += infoBarSize;
				size.y -= infoBarSize;
			}

			ImGui::SetNextWindowPos(pos);
			ImGui::SetNextWindowSize(size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
				| ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		if (opt_flags & ImGuiDockNodeFlags_DockSpace)
			window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("MyDockspace", &p_open, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		ImGuiID DockspaceID = ImGui::GetID("MyDockspace");

		if (!ImGui::DockBuilderGetNode(DockspaceID))
		{
			ImGui::DockBuilderRemoveNode(DockspaceID); // Clear out existing layout
			ImGui::DockBuilderAddNode(DockspaceID); // Add empty node
			ImGui::DockBuilderSetNodeSize(DockspaceID, ImGui::GetIO().DisplaySize);

			ImGuiID dock_main_id = DockspaceID;
			ImGuiID DockBottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.3f, nullptr, &dock_main_id);
			ImGuiID DockLeft = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.2f, nullptr, &dock_main_id);
			ImGuiID DockRight = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.20f, nullptr, &dock_main_id);

			ImGuiID DockLeftChild = ImGui::DockBuilderSplitNode(DockLeft, ImGuiDir_Down, 0.875f, nullptr, &DockLeft);
			ImGuiID DockRightChild = ImGui::DockBuilderSplitNode(DockRight, ImGuiDir_Down, 0.875f, nullptr, &DockRight);
			ImGuiID DockingLeftDownChild = ImGui::DockBuilderSplitNode(DockLeftChild, ImGuiDir_Down, 0.06f, nullptr, &DockLeftChild);
			ImGuiID DockingRightDownChild = ImGui::DockBuilderSplitNode(DockRightChild, ImGuiDir_Down, 0.06f, nullptr, &DockRightChild);

			ImGuiID DockBottomChild = ImGui::DockBuilderSplitNode(DockBottom, ImGuiDir_Down, 0.2f, nullptr, &DockBottom);
			ImGuiID DockingBottomLeftChild = ImGui::DockBuilderSplitNode(DockBottomChild, ImGuiDir_Left, 0.5f, nullptr, &DockBottomChild);
			ImGuiID DockingBottomRightChild = ImGui::DockBuilderSplitNode(DockBottomChild, ImGuiDir_Right, 0.5f, nullptr, &DockBottomChild);

			ImGuiID DockMiddle = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.8f, nullptr, &dock_main_id);

			ImGui::DockBuilderDockWindow("Scene", DockMiddle);
			ImGui::DockBuilderDockWindow("Display", DockMiddle);

			ImGui::DockBuilderDockWindow("Properties", DockRight);
			ImGui::DockBuilderDockWindow("Console", DockingBottomLeftChild);
			ImGui::DockBuilderDockWindow("Assets", DockingBottomRightChild);
			ImGui::DockBuilderDockWindow("GraphicsInfo", DockLeft);
			ImGui::DockBuilderDockWindow("ApplicationInfo", DockLeft);
			ImGui::DockBuilderDockWindow("Hierarchy", DockLeft);

			ImGui::DockBuilderFinish(DockspaceID);
		}

		// Dockspace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGui::DockSpace(DockspaceID, ImVec2(0.0f, 0.0f), opt_flags);
		}
	}

	auto Editor::endDockSpace() -> void
	{
		ImGui::End();
	}

	auto Editor::loadCachedScene() -> void
	{
	}

	auto Editor::cacheScene() -> void
	{
	}

	auto Editor::onSceneCreated(Scene* scene) -> void
	{
		Application::onSceneCreated(scene);

		for (auto & wind : editorWindows)
		{
			wind.second->onSceneCreated(scene);
		}

	}

	auto Editor::getIconFontIcon(const std::string& filePath) -> const char*
	{
		if (StringUtils::isLuaFile(filePath)) {
			return ICON_MDI_LANGUAGE_LUA;
		}
		else if (StringUtils::isCSharpFile(filePath)) {
			return ICON_MDI_LANGUAGE_CSHARP;
		}
		else if (StringUtils::isTextFile(filePath))
		{
			return ICON_MDI_FILE_XML;
		}
		else if (StringUtils::isModelFile(filePath))
		{
			return ICON_MDI_SHAPE;
		}
		else if (StringUtils::isAudioFile(filePath))
		{
			return ICON_MDI_FILE_MUSIC;
		}
		else if (StringUtils::isTextureFile(filePath))
		{
			return ICON_MDI_FILE_IMAGE;
		}
		else if (StringUtils::isSceneFile(filePath)) 
		{
			return ICON_MDI_LEAF_MAPLE;
		}
		
		return ICON_MDI_FILE;
	}

	auto Editor::openFile(const std::string& filePath) -> void
	{
		if (StringUtils::isTextFile(filePath))
		{
			LOGW("OpenFile file : {0} did not implement", filePath);
		}
		else if (StringUtils::isModelFile(filePath))
		{
			auto name = StringUtils::getFileNameWithoutExtension(filePath);
			auto modelEntity = sceneManager->getCurrentScene()->createEntity(name);
			auto& model = modelEntity.addComponent<Model>(filePath);

			if (model.resource->getMeshes().size() == 1) 
			{
				modelEntity.addComponent<MeshRenderer>(model.resource->getMeshes().begin()->second);
			}
			else 
			{
				for (auto & mesh : model.resource->getMeshes())
				{
					auto child = sceneManager->getCurrentScene()->createEntity(mesh.first);
					child.addComponent<MeshRenderer>(mesh.second);
					child.setParent(modelEntity);
				}
			}

			model.type = PrimitiveType::File;
			selectedNode = modelEntity.getHandle();
		}
		else if (StringUtils::isAudioFile(filePath))
		{
			LOGW("OpenFile file : {0} did not implement", filePath);
		}
		else if (StringUtils::isSceneFile(filePath))
		{
			sceneManager->addSceneFromFile(filePath);
			sceneManager->switchScene(filePath);
		}
		else if (StringUtils::isTextureFile(filePath))
		{
			LOGW("OpenFile file : {0} did not implement", filePath);
		}
	}

	auto Editor::drawGrid() -> void
	{
		debugRender.drawLine(
			{ -5000,0,0 }, { 5000,0,0 }, {1,0,0,1.f}
		);//x
		debugRender.drawLine(
			{ 0,-5000,0 }, { 0,5000,0 }, { 0,1,0,1.f }
		);//y
		debugRender.drawLine(
			{ 0,0,-5000 }, { 0,0,5000 }, { 0,0,1,1.f }
		);//z
	}

	auto Editor::getIcon(FileType type)->Quad2D*
	{
		if (auto iter = cacheIcons.find(type); iter != cacheIcons.end()) {
			return textureAtlas->addSprite(iter->second);
		}
		return textureAtlas->addSprite(cacheIcons[FileType::Normal]);
	}

	auto Editor::processIcons() -> void
	{
		std::array<std::string, static_cast<int32_t>(FileType::Length)> files =
		{
			"editor-icons/icons8-file-100.png",
			"editor-icons/icons8-folder-100.png",
			"editor-icons/icons8-image-file-100.png",
			"editor-icons/icons8-object-100.png",
			"editor-icons/icons8-fbx-100.png",
			"editor-icons/icons8-obj-100.png",
			"editor-icons/icons8-document-100.png",
			"editor-icons/icons8-document-100.png",
			"editor-icons/icons8-dll-80.png",
			"editor-icons/icons8-maple-leaf-100.png",
			"editor-icons/icons8-mp3-100.png",
			"editor-icons/icons8-ogg-100.png",
			"editor-icons/icons8-aac-100.png",
			"editor-icons/icons8-wav-100.png",
			"editor-icons/icons8-ttf-100.png",
			"editor-icons/icons8-cs-80.png",
		};

		textureAtlas = std::make_shared<TextureAtlas>(4096, 4096);
		int32_t i = 0;
		for (auto & str : files)
		{
			cacheIcons[static_cast<FileType>(i++)] = str;
			textureAtlas->addSprite(str);
		}
	}

	auto Editor::addPlugin(EditorPlugin* plugin) -> void
	{
		plugins.emplace_back(std::unique_ptr<EditorPlugin>(plugin));
	}

	auto Editor::addFunctionalPlugin(const std::function<void(Editor*)>& callback) -> void
	{
		plugins.emplace_back(std::make_unique<FunctionalPlugin>(callback));
	}

	auto Editor::clickObject(const Ray& ray) -> void
	{
		auto& registry = getSceneManager()->getCurrentScene()->getRegistry();
	
		float closestDist = INFINITY;
		entt::entity closestEntity = entt::null;

		auto group = registry.group<MeshRenderer>(entt::get<Transform>);
		for (auto entity : group)
		{
			const auto& [mesh, trans] = group.get<MeshRenderer, Transform>(entity);
			if (mesh.getMesh() != nullptr)
			{
				auto& worldTransform = trans.getWorldMatrix();

				auto bbCopy = mesh.getMesh()->getBoundingBox()->transform(worldTransform);
				float dist = ray.hit(bbCopy);
				LOGI("dist {0} {1}", (int32_t)entity, dist);
				if (dist < INFINITY && dist < closestDist)
				{
					closestDist = dist;
					closestEntity = entity;
				}
			}
		}

	
		static auto lastClick = timer.current();
		
		if (selectedNode != entt::null && selectedNode == closestEntity)
		{
			if (timer.elapsed(timer.current(), lastClick) / 1000000.f < 1.f)
			{
				auto& trans = registry.get<Transform>(selectedNode);
				auto& model = registry.get<MeshRenderer>(selectedNode);
				if (auto mesh = model.getMesh(); mesh != nullptr) {
					auto bb = mesh->getBoundingBox()->transform(trans.getWorldMatrix());
					focusCamera(trans.getWorldPosition(), glm::length(bb.max - bb.min));
				}
			}
			else
			{
				closestEntity = entt::null;
			}

			lastClick = timer.current();
			selectedNode = closestEntity;
			return;
		}
		auto spriteGroup = registry.group<Sprite>(entt::get<Transform>);

		for (auto entity : spriteGroup)
		{
			const auto& [sprite, trans] = spriteGroup.get<Sprite, Transform>(entity);
			auto& worldTransform = trans.getWorldMatrix();
			const glm::vec2& min = sprite.getQuad().getOffset();
			glm::vec2 max = min + glm::vec2{ sprite.getQuad().getWidth(), sprite.getQuad().getHeight() };
			BoundingBox bb{ Rect2D(min,max)};
			bb.transform(trans.getWorldMatrix());

			float dist = ray.hit(bb);

			if (dist < INFINITY && dist < closestDist)
			{
				closestDist = dist;
				closestEntity = entity;
			}
		}

		auto animSpriteGroup = registry.group<AnimatedSprite>(entt::get<Transform>);

		for (auto entity : animSpriteGroup)
		{
			const auto& [sprite, trans] = animSpriteGroup.get<AnimatedSprite, Transform>(entity);
			auto& worldTransform = trans.getWorldMatrix();
			const glm::vec2& min = sprite.getQuad().getOffset();
			glm::vec2 max = min + glm::vec2{ sprite.getQuad().getWidth(), sprite.getQuad().getHeight() };
			BoundingBox bb{ Rect2D(min,max) };
			bb = bb.transform(trans.getWorldMatrix());

			float dist = ray.hit(bb);

			if (dist < INFINITY && dist < closestDist)
			{
				closestDist = dist;
				closestEntity = entity;
			}
		}


		if (selectedNode != entt::null && selectedNode == closestEntity)
		{
			auto& trans = registry.get<Transform>(selectedNode);
			Sprite * sprite = registry.try_get<Sprite>(selectedNode);
			if (sprite == nullptr) {
				sprite = registry.try_get<AnimatedSprite>(selectedNode);
			}
			if (sprite != nullptr) {
				const glm::vec2& min = sprite->getQuad().getOffset();
				const glm::vec2 max = min + glm::vec2{ sprite->getQuad().getWidth(), sprite->getQuad().getHeight() };
				BoundingBox bb(Rect2D(min, max));
				bb = bb.transform(trans.getWorldMatrix());
				focusCamera(trans.getWorldPosition(), glm::length(bb.max - bb.min));
			}
		}

		selectedNode = closestEntity;

	}

	auto Editor::focusCamera(const glm::vec3& point, float distance, float speed /*= 1.0f*/) -> void
	{
		if (camera->isOrthographic())
		{
			editorCameraTransform.setLocalPosition(point + glm::vec3{0,0,0.1});
		}
		else
		{
			cameraDestination = point + editorCameraTransform.getForwardDirection() * distance;

			editorCameraTransform.setLocalPosition(cameraDestination);
		}
	}

	auto Editor::getScreenRay(int32_t x, int32_t y, Camera* camera, int32_t width, int32_t height) ->Ray
	{
		if (!camera)
			return {};

		float screenX = (float)x / (float)width;
		float screenY = (float)y / (float)height;

		bool flipY = true;

		return camera->sendRay(screenX, screenY, glm::inverse(editorCameraTransform.getWorldMatrix()), flipY);

	}

}

#if !defined(EDITOR_STATIC)
	Maple::Application * createApplication()
	{
		return new Maple::Editor(new Maple::DefaultDelegate());
	}
#endif