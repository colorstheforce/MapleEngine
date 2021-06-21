

#include "Light.h"
#include <imgui.h>
#include "ImGui/ImGuiHelpers.h"

namespace Maple
{

	std::string lightTypeToString(LightType type)
	{
		switch (type)
		{
		case LightType::DirectionalLight:
			return "Directional Light";
		case LightType::SpotLight:
			return "Spot Light";
		case LightType::PointLight:
			return "Point Light";
	/*	case LightType::GlobalDirectionalLight:
			return "GlobalDirectionalLight";*/
		default:
			return "ERROR";
		}
	}

	int32_t stringToLightType(const std::string& type)
	{
		if (type == "Directional")
			return int32_t(LightType::DirectionalLight);

		if (type == "Point")
			return int32_t(LightType::PointLight);

		if (type == "Spot")
			return int32_t(LightType::SpotLight);

		/*if (type == "GlobalDirectionalLight")
			return int32_t(LightType::GlobalDirectionalLight);*/


		return 0.0f;
	}


	Light::Light(const glm::vec3 & direction, const glm::vec4& color, float intensity, const LightType& type, const glm::vec3& position, float radius, float angle)
	{
		lightData.direction = { direction,1.f };
		lightData.color = color;
		lightData.position = { position,1.f };
		lightData.intensity = intensity;
		lightData.radius = radius;
		lightData.type = static_cast<float>(type);
		lightData.angle = angle;
	}
	
	auto Light::onImGui() -> void
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Columns(2);
		ImGui::Separator();

		if (static_cast<LightType>(lightData.type) != LightType::DirectionalLight)
			ImGuiHelper::property("Radius", lightData.radius, 1.0f, 100.0f);

		ImGuiHelper::property("Color", lightData.color, true, ImGuiHelper::PropertyFlag::ColorProperty);
		ImGuiHelper::property("Intensity", lightData.intensity, 0.0f, 1.0f);

		if (static_cast<LightType>(lightData.type) == LightType::SpotLight)
			ImGuiHelper::property("Angle", lightData.angle, -1.0f, 1.0f);
		

		ImGuiHelper::property("Show Frustum", showFrustum);

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Light Type");
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);


		const char* lights[] = { "Directional Light","Spot Light","Point Light"};
		auto currLight = lightTypeToString(LightType(int32_t(lightData.type)));

		if (ImGui::BeginCombo("LightType", currLight.c_str(), 0))
		{
			for (auto n = 0; n < 3; n++)
			{
				if (ImGui::Selectable(lights[n]))
				{
					lightData.type = n;
				}
			}
			ImGui::EndCombo(); 
		}

		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::PopStyleVar();
	}

	
};