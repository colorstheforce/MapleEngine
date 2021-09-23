//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "DisplayZeroWindow.h"
#include "Engine/Interface/Texture.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Event/WindowEvent.h"
#include "Devices/Input.h"
#include "Engine/Camera.h"
#include "ImGui/ImGuiHelpers.h"
#include "Editor.h"
#include "Scene/Scene.h"
#include "Engine/GBuffer.h"
#include "Math/MathUtils.h"
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include "IconsMaterialDesignIcons.h"
#include "imgui_internal.h"

namespace Maple 
{
	const ImVec4 SelectedColor(0.28f, 0.56f, 0.9f, 1.0f);
	DisplayZeroWindow::DisplayZeroWindow()
	{
		title = "Display";
	}

	auto DisplayZeroWindow::onImGui() -> void
	{
		auto& editor = *static_cast<Editor*>(Application::get());

		ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 5.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
		auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		ImGui::SetNextWindowBgAlpha(0.0f);
		auto currentScene = Application::get()->getSceneManager()->getCurrentScene();
		if (ImGui::Begin(title.c_str(), &active, flags)) 
		{
			Camera* camera = nullptr;
			auto& registry = currentScene->getRegistry();
			auto cameraView = registry.view<Camera>();
			if (!cameraView.empty())
			{
				camera = &registry.get<Camera>(cameraView.front());
			}

			if (camera != nullptr) 
			{
				ImVec2 offset = ImGui::GetCursorPos();
				drawToolBar();
				ImGuizmo::SetDrawlist();
				auto sceneViewSize = ImGui::GetWindowContentRegionMax() - ImGui::GetWindowContentRegionMin() - offset / 2.0f;// - offset * 0.5f;
				auto sceneViewPosition = ImGui::GetWindowPos() + offset;

				sceneViewSize.x -= static_cast<int>(sceneViewSize.x) % 2 != 0 ? 1.0f : 0.0f;
				sceneViewSize.y -= static_cast<int>(sceneViewSize.y) % 2 != 0 ? 1.0f : 0.0f;

				if (!freeAspect)
				{
					float heightNeededForAspect = sceneViewSize.x / aspect;

					if (heightNeededForAspect > sceneViewSize.y)
					{
						sceneViewSize.x = sceneViewSize.y * aspect;
						float xOffset = ((ImGui::GetContentRegionAvail() - sceneViewSize) * 0.5f).x;
						sceneViewPosition.x += xOffset;
						ImGui::SetCursorPos({ ImGui::GetCursorPosX() + xOffset, ImGui::GetCursorPosY() });
						offset.x += xOffset;
					}
					else
					{
						sceneViewSize.y = sceneViewSize.x / aspect;
						float yOffset = ((ImGui::GetContentRegionAvail() - sceneViewSize) * 0.5f).y;
						sceneViewPosition.y += yOffset;

						ImGui::SetCursorPos({ ImGui::GetCursorPosX(), ImGui::GetCursorPosY()  + yOffset });
						offset.y += yOffset;
					}
				}

				float asp = static_cast<float>(sceneViewSize.x) / static_cast<float>(sceneViewSize.y);

				if (!MathUtils::equals(asp, camera->getAspectRatio()))
				{
					camera->setAspectRatio(asp);
				}

				resize(static_cast<uint32_t>(sceneViewSize.x), static_cast<uint32_t>(sceneViewSize.y));
				if (camera != nullptr) {
					ImGuiHelper::image(previewTexture.get(), { static_cast<uint32_t>(sceneViewSize.x), static_cast<uint32_t>(sceneViewSize.y) });
				}

			//	ImGuizmo::SetRect(sceneViewPosition.x, sceneViewPosition.y, sceneViewSize.x, sceneViewSize.y);
				ImGui::GetWindowDrawList()->PushClipRect(sceneViewPosition, { sceneViewSize.x + sceneViewPosition.x, sceneViewSize.y + sceneViewPosition.y - 2.0f });;

				if (editor.isSceneActive() && !ImGuizmo::IsUsing() && Input::getInput()->isMouseClicked(KeyCode::MouseKey::ButtonLeft))
				{
					auto clickPos = Input::getInput()->getMousePosition() - glm::vec2(sceneViewPosition.x, sceneViewPosition.y);
				}
			}
			else 
			{

			}
		}
		else 
		{
			this->width =0;
			this->height =	0;
		}
		ImGui::End();
		ImGui::PopStyleVar(2);
	}

	static float stringToAspect(const std::string& aspect)
	{
		if (aspect == "16:10")
		{
			return 16.0f / 10.0f;
		}
		else if (aspect == "16:9")
		{
			return 16.0f / 9.0f;
		}
		else if (aspect == "4:3")
		{
			return 4.0f / 3.0f;
		}
		else if (aspect == "3:2")
		{
			return 3.0f / 2.0f;
		}
		else if (aspect == "9:16")
		{
			return 9.0f / 16.0f;
		}
		else
		{
			return 1.0f;
		}
	}


	static std::string aspectToString(float aspect)
	{
		if (MathUtils::equals(aspect, 16.0f / 10.0f))
		{
			return "16:10";
		}
		else if (MathUtils::equals(aspect, 16.0f / 9.0f))
		{
			return "16:9";
		}
		else if (MathUtils::equals(aspect, 4.0f / 3.0f))
		{
			return "4:3";
		}
		else if (MathUtils::equals(aspect, 3.0f / 2.0f))
		{
			return "3:2";
		}
		else if (MathUtils::equals(aspect, 9.0f / 16.0f))
		{
			return "9:16";
		}
		else
		{
			return "Unsupported";
		}
	}

	auto DisplayZeroWindow::resize(uint32_t width, uint32_t height) -> void
	{
		bool resized = false;
		auto& editor = *static_cast<Editor*>(Application::get());
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
			VulkanContext::get()->waiteIdle();
			previewTexture->buildTexture(TextureFormat::RGBA8, width, height, false, false, false);
			for (auto & r : Application::get()->getRenderManagers())
			{
				if (!r->isEditor()) {
					r->setRenderTarget(previewTexture, false);
					r->onResize(width, height);
				}
			}
			VulkanContext::get()->waiteIdle();
		}
	}

	auto DisplayZeroWindow::drawToolBar() -> void
	{
		ImGui::Indent();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	
		static std::string supportedAspects[] = { "Free Aspect", "16:10", "16:9", "4:3", "3:2", "9:16" };

		if (ImGui::Button("Aspect " ICON_MDI_CHEVRON_DOWN))
			ImGui::OpenPopup("AspectPopup");
		if (ImGui::BeginPopup("AspectPopup"))
		{
			std::string currentAspect = freeAspect ? "Free Aspect" : aspectToString(aspect);

			for (int n = 0; n < 6; n++)
			{
				bool selected = (currentAspect == supportedAspects[n]);
				if (ImGui::Checkbox(supportedAspects[n].c_str(), &selected))
				{
					if (supportedAspects[n] == "Free Aspect")
					{
						freeAspect = selected;
					}
					else
					{
						freeAspect = false;
						aspect = stringToAspect(supportedAspects[n]);
					}
				}
			}
			ImGui::EndPopup();
		}

		ImGui::PopStyleColor();
		ImGui::Unindent();
	}

};

