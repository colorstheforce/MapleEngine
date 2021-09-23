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
#include "Scene/Component/Sprite.h"
#include "Scene/Component/Light.h"
#include "Scene/Component/Transform.h"
#include "Scene/Component/CameraControllerComponent.h"
#include "Scene/Component/MeshRenderer.h"
#include "Engine/GBuffer.h"
#include "Engine/Renderer/RenderManager.h"
#include "Engine/Renderer/ShadowRenderer.h"
#include "Engine/Renderer/OmniShadowRenderer.h"
#include "Engine/Renderer/PreProcessRenderer.h"

#include "Scripts/Mono/MonoScript.h"
#include "Scripts/Mono/MonoComponent.h"
#include "Scripts/Mono/MonoSystem.h"

#include "Others/StringUtils.h"
#include "Engine/Camera.h"
#include "ImGui/ImGuiHelpers.h"
#include "Engine/Mesh.h"
#include "Engine/Material.h"


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


	auto textureWidget(const char* label, Material* material, std::shared_ptr<Texture2D> tex, float& usingMapProperty, glm::vec4& colorProperty, const std::function<void(const std::string&)>& callback) -> void
	{
		if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			ImGui::Columns(2);
			ImGui::Separator();

			ImGui::AlignTextToFramePadding();

			const ImGuiPayload* payload = ImGui::GetDragDropPayload();

			auto min = ImGui::GetCursorPos();
			auto max = min + ImVec2{ 64,64 } + ImGui::GetStyle().FramePadding;

			bool hoveringButton = ImGui::IsMouseHoveringRect(min, max, false);
			bool showTexture = !(hoveringButton && (payload != NULL && payload->IsDataType("AssetFile")));
			if (tex && showTexture)
			{
				const bool flipImage = false;
				if (ImGui::ImageButton(tex->getHandle(), ImVec2{ 64,64 }, ImVec2(0.0f, flipImage ? 1.0f : 0.0f), ImVec2(1.0f, flipImage ? 0.0f : 1.0f)))
				{

				}

				if (ImGui::IsItemHovered() && tex)
				{
					ImGui::BeginTooltip();
					ImGui::Image(tex->getHandle(), ImVec2(256, 256), ImVec2(0.0f, flipImage ? 1.0f : 0.0f), ImVec2(1.0f, flipImage ? 0.0f : 1.0f));
					ImGui::EndTooltip();
				}
			}
			else
			{
				if (ImGui::Button(tex ? "" : "Empty", ImVec2{ 64,64 }))
				{
					/*Lumos::Editor::GetEditor()->GetFileBrowserWindow().Open();
					Lumos::Editor::GetEditor()->GetFileBrowserWindow().SetCallback(callback);*/
				}
			}

			if (payload != NULL && payload->IsDataType("AssetFile"))
			{
				auto filePath = std::string(reinterpret_cast<const char*>(payload->Data));
				if (StringUtils::isTextureFile(filePath))
				{
					if (ImGui::BeginDragDropTarget())
					{
						// Drop directly on to node and append to the end of it's children list.
						if (ImGui::AcceptDragDropPayload("AssetFile"))
						{
							callback(filePath);
							ImGui::EndDragDropTarget();

							ImGui::Columns(1);
							ImGui::Separator();
							ImGui::PopStyleVar();

							ImGui::TreePop();
							return;
						}

						ImGui::EndDragDropTarget();
					}
				}
			}

			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			ImGui::TextUnformatted(tex ? tex->getFilePath().c_str() : "No Texture");
			if (tex)
			{
				ImGuiHelper::tooltip(tex->getFilePath().c_str());
				ImGui::Text("%u x %u", tex->getWidth(), tex->getHeight());
				ImGui::Text("Mip Levels : %u", tex->getMipmapLevel());
			}
			ImGui::PopItemWidth();
			ImGui::NextColumn();

			ImGuiHelper::property("Use Map", usingMapProperty, 0.0f, 1.0f);
			ImGuiHelper::property("Color", colorProperty, 0.0f, 1.0f, false, ImGuiHelper::PropertyFlag::ColorProperty);

			ImGui::Columns(1);
			ImGui::PopStyleVar();

			ImGui::TreePop();
		}
	}


	template<>
	void ComponentEditorWidget<Sprite>(entt::registry& reg, entt::registry::entity_type e)
	{
		auto& sprite = reg.get<Sprite>(e);

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Columns(2);
		ImGui::Separator();

		/*auto pos = sprite.getQuad().getPosition();
		if (ImGuiHelper::property("Position", pos))
			sprite.getQuad().setPosition(pos);

		
	
		auto scale = sprite.getQuad().getScale();
		if (ImGuiHelper::property("Scale", scale))
			sprite.getQuad().setScale(scale);

	
		auto color = sprite.getQuad().getColor();
		if (ImGuiHelper::property("Colour", color,-1,1,false, ImGuiHelper::PropertyFlag::ColorProperty))
			sprite.getQuad().setColor(color);*/

		ImGui::Columns(1);
	
		if (ImGui::TreeNode("Texture"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			ImGui::Columns(2);
			ImGui::Separator();
			

			ImGui::AlignTextToFramePadding();
			auto tex = sprite.getQuad().getTexture();

			ImVec2 imageButtonSize(64, 64);

			auto callback = std::bind(&Sprite::setTextureFromFile, &sprite, std::placeholders::_1);
			const ImGuiPayload* payload = ImGui::GetDragDropPayload();
			auto min = ImGui::GetCursorPos();
			auto max = min + imageButtonSize + ImGui::GetStyle().FramePadding;

			bool hoveringButton = ImGui::IsMouseHoveringRect(min, max, false);
			bool showTexture = !(hoveringButton && (payload != NULL && payload->IsDataType("AssetFile")));
			bool flipImage = false;
			if (tex && showTexture)
			{
				if (ImGui::ImageButton(tex->getHandle(), imageButtonSize, ImVec2(0.0f, flipImage ? 1.0f : 0.0f), ImVec2(1.0f, flipImage ? 0.0f : 1.0f)))
				{
				
				}

				if (ImGui::IsItemHovered() && tex)
				{
					ImGui::BeginTooltip();
					ImGui::Image(tex->getHandle(), ImVec2(256, 256), ImVec2(0.0f, flipImage ? 1.0f : 0.0f), ImVec2(1.0f, flipImage ? 0.0f : 1.0f));
					ImGui::EndTooltip();
				}
			}
			else
			{
				if (ImGui::Button(tex ? "" : "Empty", imageButtonSize))
				{
					
					
				}
			}

			if (payload != NULL && payload->IsDataType("AssetFile"))
			{
				auto filePath = std::string(reinterpret_cast<const char*>(payload->Data));
				if (StringUtils::isTextureFile(filePath))
				{
					if (ImGui::BeginDragDropTarget())
					{
						// Drop directly on to node and append to the end of it's children list.
						if (ImGui::AcceptDragDropPayload("AssetFile"))
						{
							callback(filePath);
							ImGui::EndDragDropTarget();

							ImGui::Columns(1);
							ImGui::Separator();
							ImGui::PopStyleVar(2);

							ImGui::TreePop();
							return;
						}

						ImGui::EndDragDropTarget();
					}
				}
			}

			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			ImGui::TextUnformatted(tex ? tex->getFilePath().c_str() : "No Texture");
			if (tex)
			{
				ImGuiHelper::tooltip(tex->getFilePath().c_str());
				ImGui::Text("%u x %u", tex->getWidth(), tex->getHeight());
				ImGui::Text("Mip Levels : %u", tex->getMipmapLevel());
			}

			ImGui::PopItemWidth();
			ImGui::NextColumn();

			ImGui::Columns(1);
			ImGui::Separator();
			ImGui::PopStyleVar();
			ImGui::TreePop();
		}

		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::PopStyleVar();
	}

	template<>
	void ComponentEditorWidget<AnimatedSprite>(entt::registry& reg, entt::registry::entity_type e)
	{
		auto& sprite = reg.get<AnimatedSprite>(e);

		ImGui::Columns(2);

		auto frame = sprite.getCurrentId();
		if (ImGuiHelper::property("Frame", frame, 0, sprite.getFrames() - 1)) {
			sprite.setCurrentFrame(frame);
		}
		bool loop = sprite.isLoop();
		if (ImGuiHelper::property("Loop", loop)) {
			sprite.setLoop(loop);
		}

		auto delay = sprite.getDelay();
		if (ImGuiHelper::inputFloat("Delay", delay)) {}

		auto timer = sprite.getTimer();
		if (ImGuiHelper::inputFloat("Timer", timer)) {}

		ImGui::Columns(1);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Separator();
		ImGui::AlignTextToFramePadding();
		auto tex = sprite.getQuad().getTexture();
		auto coords = sprite.getQuad().getTexCoords();

		auto& color = sprite.getQuad().getColor();

		if (tex)
		{
			
			auto w = (float)sprite.getQuad().getWidth();
			auto h = (float)sprite.getQuad().getHeight();

			if (w > 256) {
				h = 256.f / w * h;
				w = 256;
			}

	
			ImGui::Image(tex->getHandle(), {w,h}, 
				ImVec2(coords[3].x, coords[3].y), 
				ImVec2(coords[1].x, coords[1].y),
				{color.r,color.g ,color.b ,color.a}
			);
		}

		ImGui::PopStyleVar();
	
	}


	template<>
	void ComponentEditorWidget<MeshRenderer>(entt::registry& reg, entt::registry::entity_type e)
	{
		auto& mesh = reg.get<MeshRenderer>(e);

		auto material = mesh.getMesh()->getMaterial();

		std::string matName = "Material";
		if (!material)
		{
			ImGui::TextUnformatted("Empty Material");
			if (ImGui::Button("Add Material", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
				mesh.getMesh()->setMaterial(std::make_shared<Material>());
		}
		else if (ImGui::TreeNodeEx(matName.c_str(), 0))
		{

			auto & prop = material->getProperties();
			auto color = glm::vec4(0.f);
			auto textures = material->getTextures();

			textureWidget("Albedo", material.get(), textures.albedo, prop.usingAlbedoMap, prop.albedoColor, std::bind(&Material::setAlbedoTexture, material, std::placeholders::_1));
			ImGui::Separator();

			textureWidget("Normal", material.get(), textures.normal, prop.usingNormalMap, color, std::bind(&Material::setNormalTexture, material, std::placeholders::_1));
			ImGui::Separator();

			textureWidget("Metallic", material.get(), textures.metallic, prop.usingMetallicMap, prop.metallicColor, std::bind(&Material::setMetallicTexture, material, std::placeholders::_1));
			ImGui::Separator();

			textureWidget("Roughness", material.get(), textures.roughness, prop.usingRoughnessMap, prop.roughnessColor, std::bind(&Material::setRoughnessTexture, material, std::placeholders::_1));
			ImGui::Separator();

			textureWidget("AO", material.get(), textures.ao, prop.usingAOMap, color, std::bind(&Material::setAOTexture, material, std::placeholders::_1));
			ImGui::Separator();

			textureWidget("Emissive", material.get(), textures.emissive, prop.usingEmissiveMap, prop.emissiveColor, std::bind(&Material::setEmissiveTexture, material, std::placeholders::_1));

			material->setMaterialProperites(prop);

			ImGui::TreePop();
		}

	}


	template<>
	void ComponentEditorWidget<Light>(entt::registry& reg, entt::registry::entity_type e)
	{
		auto& light = reg.get<Light>(e);
		light.onImGui();
	}


	template<>
	void ComponentEditorWidget<MonoComponent>(entt::registry& reg, entt::registry::entity_type e)
	{
		auto& mono = reg.get<MonoComponent>(e);
		ImGui::PushID("add c# script");
		static std::string path = "scripts/";
		ImGui::PushItemWidth(-1);
		if (ImGui::BeginCombo("", "", 0))
		{
			for (const auto& entry : std::filesystem::directory_iterator(path))
			{
				bool isDir = std::filesystem::is_directory(entry);
				if (StringUtils::isCSharpFile(entry.path().string()))
				{
					if (ImGui::Selectable(entry.path().string().c_str()))
					{
						mono.addScript(entry.path().string(), Application::get()->getSystemManager()->getSystem<MonoSystem>());
					}
				
				}
			}
			ImGui::EndCombo();
		}
		ImGui::PopID();
	}


	template<>
	void ComponentEditorWidget<Environment>(entt::registry& reg, entt::registry::entity_type e)
	{
		auto& env = reg.get<Environment>(e);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Columns(2);
		ImGui::Separator();

	
		auto label = env.getFilePath();
		if (ImGuiHelper::property("File", label, true))
		{
			
		}

		if (ImGui::BeginDragDropTarget())
		{
			auto data = ImGui::AcceptDragDropPayload("AssetFile", ImGuiDragDropFlags_None);
			if (data)
			{
				std::string file = (char*)data->Data;
				if (StringUtils::isTextureFile(file)) {
					env.init(file);
				}
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::Columns(1);
		ImGui::Separator();

		/*ImGui::Columns(1);
		ImGui::Separator();

		if (env.getEquirectangularMap()) {
			ImGuiHelper::image(
				env.getEquirectangularMap().get()
				, { 100,100 });
		}
*/

		ImGui::PopStyleVar();
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
		if (ImGuiHelper::inputFloat("Scale", scale, 0.0f, 10000.f))
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
		auto& editor = static_cast<Editor&>(*Application::get());
		auto& registry = editor.getSceneManager()->getCurrentScene()->getRegistry();
		auto selected = editor.getSelected();


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

		auto& editor = static_cast<Editor&>(*Application::get());
		auto& iconMap = editor.getComponentIconMap();

#define TRIVIAL_COMPONENT(ComponentType,show,showName) \
	{ \
		std::string name; \
		if(iconMap.find(typeid(ComponentType).hash_code()) != iconMap.end()) \
			name += iconMap[typeid(ComponentType).hash_code()]; \
        else \
            name += iconMap[typeid(Editor).hash_code()]; \
		name += "\t"; \
		if(showName != std::string("")){ \
			name+=showName; \
		} else {\
			name += ###ComponentType; \
		}\
		enttEditor.registerComponent<ComponentType>(name,show); \
	}

		TRIVIAL_COMPONENT(Transform, true,"");
		TRIVIAL_COMPONENT(Light, true, "");
		TRIVIAL_COMPONENT(Camera, true, "");
		TRIVIAL_COMPONENT(CameraControllerComponent, true, "Camera Controller");
		TRIVIAL_COMPONENT(Environment, true,"");
		TRIVIAL_COMPONENT(Sprite, true, "");
		TRIVIAL_COMPONENT(AnimatedSprite, true, "Animation Sprite");
		TRIVIAL_COMPONENT(MeshRenderer, false,"Mesh Renderer");

		MM::EntityEditor<entt::entity>::ComponentInfo info;
		info.hasChildren = true;


		info.childrenDraw = [](entt::registry & registry,entt::entity ent) {
			auto mono = registry.try_get<MonoComponent>(ent);

			if (mono) {
				for (auto & script : mono->getScripts())
				{
					ImGui::PushID(script.first.c_str());
					if (ImGui::Button("-")) {
						mono->remove(script.first);
						ImGui::PopID();
						continue;
					}
					else {
						ImGui::SameLine();
					}

					if (ImGui::CollapsingHeader(script.second->getClassNameInEditor().c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
						ImGui::Indent(30.f);
						ImGui::PushID("Widget");
						//	ci.widget(registry, e);

						ImGui::PopID();
						ImGui::Unindent(30.f);
					}
					ImGui::PopID();
				}
			}
		};
		enttEditor.registerComponent<MonoComponent>(info);

		enttEditor.acceptFile = [&](const std::string& fileName, entt::registry& registry, entt::entity & ent) {
			if (StringUtils::isCSharpFile(fileName)) {
				auto & mono = registry.get_or_emplace<MonoComponent>(ent);
				mono.setEntity(ent);
				mono.addScript(fileName, Application::get()->getSystemManager()->getSystem<MonoSystem>());
			}
		};
	}

};

