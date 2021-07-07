//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "SceneWindow.h"
#include "Engine/Interface/Texture.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Event/WindowEvent.h"
#include "Devices/Input.h"
#include "Engine/Camera.h"
#include "ImGui/ImGuiHelpers.h"
#include "Editor.h"
#include "Scene/Scene.h"
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include "IconsMaterialDesignIcons.h"

#include "imgui_internal.h"

namespace Maple 
{
	const ImVec4 SelectedColor(0.28f, 0.56f, 0.9f, 1.0f);
	SceneWindow::SceneWindow()
	{
		title = "Scene";
	}

	auto SceneWindow::onImGui() -> void
	{
		
		auto& editor = *static_cast<Editor*>(app);


		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		ImGui::SetNextWindowBgAlpha(0.0f);

		auto currentScene = app->getSceneManager()->getCurrentScene();
		ImGui::Begin(title.c_str(), &active, flags);
		Camera* camera = nullptr;
		Transform* transform = nullptr;

		bool gameView = false;

		if (editor.getEditorState() == EditorState::Preview && !showCamera)
		{
			camera = editor.getCamera().get();
			transform = &editor.getEditorCameraTransform();
			currentScene->setOverrideCamera(camera);
			currentScene->setOverrideTransform(transform);
		}
		else
		{
			gameView = true;
			currentScene->setOverrideCamera(nullptr);
			currentScene->setOverrideTransform(nullptr);

			auto& registry = currentScene->getRegistry();
			auto cameraView = registry.view<Camera>();
			if (!cameraView.empty())
			{
				camera = &registry.get<Camera>(cameraView.front());
			}
		}

		ImVec2 offset = { 0.0f, 0.0f };

		drawToolBar();

		ImGuizmo::SetDrawlist();

		auto sceneViewSize = ImGui::GetWindowContentRegionMax() - ImGui::GetWindowContentRegionMin() - offset / 2.0f;// - offset * 0.5f;
		auto sceneViewPosition = ImGui::GetWindowPos() + offset;

		sceneViewSize.x -= static_cast<int>(sceneViewSize.x) % 2 != 0 ? 1.0f : 0.0f;
		sceneViewSize.y -= static_cast<int>(sceneViewSize.y) % 2 != 0 ? 1.0f : 0.0f;

		resize(static_cast<uint32_t>(sceneViewSize.x), static_cast<uint32_t>(sceneViewSize.y));

		ImGuiHelper::image(previewTexture.get(), { static_cast<uint32_t>(sceneViewSize.x), static_cast<uint32_t>(sceneViewSize.y) });

		auto windowSize = ImGui::GetWindowSize();
		ImVec2 minBound = sceneViewPosition;

		ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y };
		bool updateCamera = ImGui::IsMouseHoveringRect(minBound, maxBound);

		editor.setSceneActive(ImGui::IsWindowFocused() && !ImGuizmo::IsUsing() && updateCamera);

		ImGuizmo::SetRect(sceneViewPosition.x, sceneViewPosition.y, sceneViewSize.x, sceneViewSize.y);
		ImGui::GetWindowDrawList()->PushClipRect(sceneViewPosition, { sceneViewSize.x + sceneViewPosition.x, sceneViewSize.y + sceneViewPosition.y - 2.0f });;

		if (editor.getEditorState() == EditorState::Preview && !showCamera && transform != nullptr)
		{
			const float* cameraViewPtr = glm::value_ptr(glm::inverse(transform->getWorldMatrix()));

		

			ImGui::GetWindowDrawList()->PushClipRect(sceneViewPosition, { sceneViewSize.x + sceneViewPosition.x, sceneViewSize.y + sceneViewPosition.y - 2.0f });

			float viewManipulateRight = sceneViewPosition.x + sceneViewSize.x;
			float viewManipulateTop = sceneViewPosition.y;

			ImGuizmo::ViewManipulate(const_cast<float*>(cameraViewPtr), 8, ImVec2(viewManipulateRight - 70, viewManipulateTop + 32), ImVec2(64, 64), 0x10101010);

			editor.onImGuizmo();

			if (editor.isSceneActive() && !ImGuizmo::IsUsing() && Input::getInput()->isMouseClicked(KeyCode::MouseKey::ButtonLeft))
			{
				auto clickPos = Input::getInput()->getMousePosition() - glm::vec2(sceneViewPosition.x, sceneViewPosition.y);
				//editor.SelectObject(editor.sendScreenRay(int32_t(clickPos.x), int32_t(clickPos.y), camera, int32_t(sceneViewSize.x), int32_t(sceneViewSize.y)));
			}
			drawGizmos(sceneViewSize.x, sceneViewSize.y, offset.x, offset.y, currentScene);
		}


		if (ImGui::BeginDragDropTarget())
		{
			auto data = ImGui::AcceptDragDropPayload("AssetFile");
			if (data)
			{
				std::string file = (char*)data->Data;
				LOGV("Receive file from assets window : {0}", file);
				editor.openFile(file);
			}
			ImGui::EndDragDropTarget();
		}


		ImGui::End();
		ImGui::PopStyleVar();


	}

	auto SceneWindow::resize(uint32_t width, uint32_t height) -> void
	{
		bool resized = false;
		auto& editor = *static_cast<Editor*>(app);
		if (this->width != width || this->height != height)
		{
			resized = true;
			this->width = width;
			this->height = height;
		}
		
		if (previewTexture == nullptr) {
			previewTexture = Texture2D::create();
		}

		if (resized) 
		{
			editor.getCamera()->setAspectRatio(width / (float)height);
			VulkanContext::get()->waiteIdle();
			previewTexture->buildTexture(TextureFormat::RGBA8, width, height, false, false, false);
			app->getRenderManager()->setRenderTarget(previewTexture, false, true);
			app->getRenderManager()->onResize(width, height,true);
			VulkanContext::get()->waiteIdle();
		}

	}

	auto SceneWindow::drawGizmos(float width, float height, float xpos, float ypos, Scene* scene) -> void
	{

	}

	auto SceneWindow::drawToolBar() -> void
	{
		ImGui::Indent();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		bool selected = false;
		auto& editor = *static_cast<Editor*>(app);
		{
			selected = editor.getImGuizmoOperation() == 4;
			if (selected)
				ImGui::PushStyleColor(ImGuiCol_Text, SelectedColor);
			ImGui::SameLine();
			if (ImGui::Button(ICON_MDI_CURSOR_DEFAULT))
				editor.setImGuizmoOperation(4);

			if (selected)
				ImGui::PopStyleColor();
			ImGuiHelper::tooltip("Select");
		}

		ImGui::SameLine();
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		ImGui::SameLine();

		{
			selected = editor.getImGuizmoOperation() == ImGuizmo::TRANSLATE;
			if (selected)
				ImGui::PushStyleColor(ImGuiCol_Text, SelectedColor);
			ImGui::SameLine();
			if (ImGui::Button(ICON_MDI_ARROW_ALL))
				editor.setImGuizmoOperation(ImGuizmo::TRANSLATE);

			if (selected)
				ImGui::PopStyleColor();
			ImGuiHelper::tooltip("Translate");
		}

		{
			selected = editor.getImGuizmoOperation() == ImGuizmo::ROTATE;
			if (selected)
				ImGui::PushStyleColor(ImGuiCol_Text, SelectedColor);

			ImGui::SameLine();
			if (ImGui::Button(ICON_MDI_ROTATE_ORBIT))
				editor.setImGuizmoOperation(ImGuizmo::ROTATE);

			if (selected)
				ImGui::PopStyleColor();
			ImGuiHelper::tooltip("Rotate");
		}

		{
			selected = editor.getImGuizmoOperation() == ImGuizmo::SCALE;
			if (selected)
				ImGui::PushStyleColor(ImGuiCol_Text, SelectedColor);

			ImGui::SameLine();
			if (ImGui::Button(ICON_MDI_ARROW_EXPAND_ALL))
				editor.setImGuizmoOperation(ImGuizmo::SCALE);

			if (selected)
				ImGui::PopStyleColor();
			ImGuiHelper::tooltip("Scale");
		}

		ImGui::SameLine();
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		ImGui::SameLine();

		{
			selected = editor.getImGuizmoOperation() == ImGuizmo::BOUNDS;
			if (selected)
				ImGui::PushStyleColor(ImGuiCol_Text, SelectedColor);

			ImGui::SameLine();
			if (ImGui::Button(ICON_MDI_BORDER_NONE))
				editor.setImGuizmoOperation(ImGuizmo::BOUNDS);

			if (selected)
				ImGui::PopStyleColor();
			ImGuiHelper::tooltip("Bounds");
		}



		ImGui::SameLine();
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		ImGui::SameLine();



		if (ImGui::Button("Gizmos " ICON_MDI_CHEVRON_DOWN))
			ImGui::OpenPopup("GizmosPopup");
		if (ImGui::BeginPopup("GizmosPopup"))
		{
			LOGW("TODO --- GizmosPopup");
			ImGui::EndPopup();
		}

		ImGui::PopStyleColor();
		ImGui::Unindent();
	}

};

