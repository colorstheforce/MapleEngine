//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "PropertiesWindow.h"
#include "Editor.h"
#include <glm/gtc/type_ptr.hpp>
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Entity.h"
#include "Scene/Component/Component.h"
#include "Scene/Component/Light.h"
#include "Scene/Component/Transform.h"
#include "Scene/Component/CameraControllerComponent.h"
#include "Scene/Component/MeshRenderer.h"
#include "Engine/GBuffer.h"
#include "Engine/Renderer/RenderManager.h"
#include "Engine/Renderer/ShadowRenderer.h"
#include "Engine/Renderer/OmniShadowRenderer.h"

#include "Engine/Camera.h"
#include "ImGui/ImGuiHelpers.h"


namespace MM
{

	using namespace Maple;

	template<>
	void ComponentEditorWidget<Transform>(entt::registry& reg, entt::registry::entity_type e)
	{
		auto& transform = reg.get<Transform>(e);

		auto rotation = glm::degrees(transform.getLocalOrientation());
		auto position = transform.getLocalPosition();
		auto scale = transform.getLocalScale();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Columns(2);
		ImGui::Separator();

		ImGui::TextUnformatted("Position");
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		if (ImGui::DragFloat3("##Position", glm::value_ptr(position), 0.05))
		{
			transform.setLocalPosition(position);
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		ImGui::TextUnformatted("Rotation");
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);


		if (ImGui::DragFloat3("##Rotation", glm::value_ptr(rotation), 0.1))
		{
			transform.setLocalOrientation(glm::radians(rotation));
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		ImGui::TextUnformatted("Scale");
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);
		if (ImGui::DragFloat3("##Scale", glm::value_ptr(scale), 0.05))
		{
			transform.setLocalScale(scale);
		}

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::PopStyleVar();
	}

	template<>
	void ComponentEditorWidget<Light>(entt::registry& reg, entt::registry::entity_type e)
	{
		auto& light = reg.get<Light>(e);
		light.onImGui();
	}

	template<>
	void ComponentEditorWidget<Camera>(entt::registry& reg, entt::registry::entity_type e)
	{
		auto& camera = reg.get<Camera>(e);

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Columns(2);
		ImGui::Separator();


		float aspect = camera.getAspectRatio();

		if (ImGuiHelper::property("Aspect", aspect, 0.0f, 10.0f))
			camera.setAspectRatio(aspect);

		float fov = camera.getFov();
		if (ImGuiHelper::property("Fov", fov, 1.0f, 120.0f))
			camera.setFov(fov);

		float near_ = camera.getNear();
		if (ImGuiHelper::inputFloat("Near", near_, 0.01, 10.f))
			camera.setNear(near_);

		float far_ = camera.getFar();
		if (ImGuiHelper::inputFloat("Far", far_, 10.0f, 1000.0f))
			camera.setFar(far_);

		float scale = camera.getScale();
		if (ImGuiHelper::property("Scale", scale, 0.0f, 100.0f))
			camera.setScale(scale);

		bool ortho = camera.isOrthographic();
		if (ImGuiHelper::property("Orthographic", ortho))
			camera.setOrthographic(ortho);


		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::PopStyleVar();

	}

	template<>
	void ComponentEditorWidget<CameraControllerComponent>(entt::registry& reg, entt::registry::entity_type e)
	{
		auto& controllerComp = reg.get<CameraControllerComponent>(e);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Columns(2);
		ImGui::Separator();

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Controller Type");
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);


		const std::array<std::string, 2> controllerTypes = { "FPS" ,"Editor" };
		std::string currentController = CameraControllerComponent::typeToString(controllerComp.getType());
		if (ImGui::BeginCombo("", currentController.c_str(), 0)) // The second parameter is the label previewed before opening the combo.
		{
			for (auto n = 0; n < controllerTypes.size(); n++)
			{
				bool isSelected = currentController == controllerTypes[n];
				if (ImGui::Selectable(controllerTypes[n].c_str(), currentController.c_str()))
				{
					controllerComp.setControllerType(CameraControllerComponent::stringToType(controllerTypes[n]));
				}
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		if (controllerComp.getController())
			controllerComp.getController()->onImGui();

		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::PopStyleVar();
	}

};


namespace Maple
{
	constexpr size_t INPUT_BUFFER = 256;
	PropertiesWindow::PropertiesWindow()
	{
		title = "Properties";
	}

	auto PropertiesWindow::onImGui() -> void
	{
		auto& editor = static_cast<Editor&>(*app);
		auto& registry = editor.getSceneManager()->getCurrentScene()->getRegistry();
		auto selected = editor.getSelected();

		auto gbuffer = editor.getRenderManager()->getGBuffer();

		if (ImGui::Begin(title.c_str(), &active))
		{
			if (selected == entt::null)
			{
				ImGui::End();
				return;
			}

			auto activeComponent = registry.try_get<ActiveComponent>(selected);
			bool active = activeComponent ? activeComponent->active : true;
			if (ImGui::Checkbox("##ActiveCheckbox", &active))
			{
				if (!activeComponent)
					registry.emplace<ActiveComponent>(selected, active);
				else
					activeComponent->active = active;
			}
			ImGui::SameLine();
			ImGui::TextUnformatted(ICON_MDI_CUBE);
			ImGui::SameLine();

			bool hasName = registry.has<NameComponent>(selected);
			std::string name;
			if (hasName)
				name = registry.get<NameComponent>(selected).name;
			else
				name = std::to_string(entt::to_integral(selected));

			static char objName[INPUT_BUFFER];
			strcpy(objName, name.c_str());

			if (ImGui::InputText("##Name", objName, IM_ARRAYSIZE(objName)))
				registry.get_or_emplace<NameComponent>(selected).name = objName;

			ImGui::Separator();

			enttEditor.renderEditor(registry, selected);

			if (ImGui::BeginDragDropTarget())
			{
				auto data = ImGui::AcceptDragDropPayload("AssetFile", ImGuiDragDropFlags_None);
				if (data)
				{
				
				}
				ImGui::EndDragDropTarget();
			}
		}
		ImGui::End();
	}

	auto PropertiesWindow::onSceneCreated(Scene* scene) -> void
	{
		enttEditor.clear();

		auto& editor = static_cast<Editor&>(*app);
		auto& iconMap = editor.getComponentIconMap();

#define TRIVIAL_COMPONENT(ComponentType,show) \
	{ \
		std::string name; \
		if(iconMap.find(typeid(ComponentType).hash_code()) != iconMap.end()) \
			name += iconMap[typeid(ComponentType).hash_code()]; \
        else \
            name += iconMap[typeid(Editor).hash_code()]; \
		name += "\t"; \
		name += ###ComponentType; \
		enttEditor.registerComponent<ComponentType>(name,show); \
	}

		TRIVIAL_COMPONENT(Transform, true);
		TRIVIAL_COMPONENT(Light, true);
		TRIVIAL_COMPONENT(Camera, true);
		TRIVIAL_COMPONENT(CameraControllerComponent, true);
		TRIVIAL_COMPONENT(MeshRenderer, false);
	}

};

