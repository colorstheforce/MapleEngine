//////////////////////////////////////////////////////////////////////////////
// This file is part of the Raven Game Engine			                    //
//////////////////////////////////////////////////////////////////////////////

#include "ImGuiHelpers.h"
#include <glm/gtc/type_ptr.hpp>
#include "Engine/Interface/Texture.h"
#include "Engine/Vulkan/VulkanTexture.h"
#include <imgui_impl_vulkan.h>

namespace Maple
{
	namespace ImGuiHelper
	{
		auto tooltip(const char* str) -> void
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextUnformatted(str);
				ImGui::EndTooltip();
			}
			ImGui::PopStyleVar();
		}

		auto property(const std::string& name, bool& value) -> bool
		{
			bool updated = false;
			ImGui::TextUnformatted(name.c_str());
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			std::string id = "##" + name;
			if (ImGui::Checkbox(id.c_str(), &value))
				updated = true;

			ImGui::PopItemWidth();
			ImGui::NextColumn();
			return updated;
		}

		auto property(const std::string& name, float& value, float min, float max, PropertyFlag flags) -> bool
		{
			bool updated = false;
			ImGui::TextUnformatted(name.c_str());
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			std::string id = "##" + name;
			if (ImGui::SliderFloat(id.c_str(), &value, min, max))
				updated = true;

			ImGui::PopItemWidth();
			ImGui::NextColumn();


			return updated;
		}

		auto property(const std::string& name, int32_t& value, int32_t min, int32_t max, PropertyFlag flags) -> bool
		{
			bool updated = false;
			ImGui::TextUnformatted(name.c_str());
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			std::string id = "##" + name;
			if (ImGui::SliderInt(id.c_str(), &value, min, max))
				updated = true;

			ImGui::PopItemWidth();
			ImGui::NextColumn();

			return updated;
		}

		auto property(const std::string& name, glm::vec2& value, PropertyFlag flags) -> bool
		{ 
			return ImGuiHelper::property(name, value, -1.0f, 1.0f, flags);
		}

		auto property(const std::string& name, glm::vec2& value, float min, float max, PropertyFlag flags) -> bool
		{
			bool updated = false;

			ImGui::TextUnformatted(name.c_str());
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			std::string id = "##" + name;
			if (ImGui::SliderFloat2(id.c_str(), glm::value_ptr(value), min, max))
				updated = true;

			ImGui::PopItemWidth();
			ImGui::NextColumn();

			return updated;
		}

		auto property(const std::string& name, glm::vec3& value, PropertyFlag flags) -> bool
		{
			return ImGuiHelper::property(name, value, -1.0f, 1.0f, flags);
		}

		auto property(const std::string& name, glm::vec3& value, float min, float max, PropertyFlag flags) -> bool
		{
			bool updated = false;

			ImGui::TextUnformatted(name.c_str());
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			std::string id = "##" + name;
			if ((int)flags & (int)PropertyFlag::ColorProperty)
			{
				if (ImGui::ColorEdit3(id.c_str(), glm::value_ptr(value), ImGuiColorEditFlags_NoInputs))
					updated = true;
			}
			else
			{
				if (ImGui::SliderFloat3(id.c_str(), glm::value_ptr(value), min, max))
					updated = true;
			}

			ImGui::PopItemWidth();
			ImGui::NextColumn();

			return updated;
		}

		auto property(const std::string& name, glm::vec4& value, bool exposeW, PropertyFlag flags) -> bool
		{
			return property(name, value, -1.0f, 1.0f, exposeW, flags);
		}

		auto property(const std::string& name, glm::vec4& value, float min /*= -1.0f*/, float max /*= 1.0f*/, bool exposeW /*= false*/, PropertyFlag flags /*= PropertyFlag::None*/) -> bool
		{
			bool updated = false;

			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(name.c_str());
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			std::string id = "##" + name;
			if ((int)flags & (int)PropertyFlag::ColorProperty)
			{
				if (ImGui::ColorEdit4(id.c_str(), glm::value_ptr(value), ImGuiColorEditFlags_NoInputs))
					updated = true;
			}
			else if ((exposeW ? ImGui::SliderFloat4(id.c_str(), glm::value_ptr(value), min, max) : ImGui::SliderFloat3(id.c_str(), glm::value_ptr(value), min, max)))
				updated = true;

			ImGui::PopItemWidth();
			ImGui::NextColumn();

			return updated;
		}

		auto property(const std::string& name, glm::vec3& value, float min /*= -1.0f*/, float max /*= 1.0f*/, bool exposeW /*= false*/, PropertyFlag flags /*= PropertyFlag::None*/) -> bool
		{
			bool updated = false;

			ImGui::TextUnformatted(name.c_str());
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			std::string id = "##" + name;
			if ((int)flags & (int)PropertyFlag::ColorProperty)
			{
				if (ImGui::ColorEdit3(id.c_str(), glm::value_ptr(value), ImGuiColorEditFlags_NoInputs))
					updated = true;
			}
			else
			{
				if (ImGui::SliderFloat3(id.c_str(), glm::value_ptr(value), min, max))
					updated = true;
			}

			ImGui::PopItemWidth();
			ImGui::NextColumn();

			return updated;
		}

		auto property(const std::string& name, std::string& value, bool disable) -> bool
		{
			bool updated = false;
			ImGui::TextUnformatted(name.c_str());
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			std::string id = "##" + name;
			static char obj[256] = {};
			memcpy(obj, value.c_str(), value.length() + 1);

			ImGuiInputTextFlags flags = ImGuiInputTextFlags_None;
			if (disable)
			{
				flags = ImGuiInputTextFlags_ReadOnly;
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(200, 200, 200)));
			}

			if (ImGui::InputText(id.c_str(), obj, 256))
			{
				updated = true;
				value = obj;
			}

			if (disable)
				ImGui::PopStyleColor();


			ImGui::PopItemWidth();
			ImGui::NextColumn();

			return updated;
		}

		auto property(const std::string& name, uint32_t& value, uint32_t min /*= 0*/, uint32_t max /*= 1*/, PropertyFlag flags /*= PropertyFlag::None*/) -> bool
		{
			bool updated = false;
			ImGui::TextUnformatted(name.c_str());
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			std::string id = "##" + name;
			if (ImGui::SliderInt(id.c_str(), (int32_t*)&value, min, max))
				updated = true;

			ImGui::PopItemWidth();
			ImGui::NextColumn();

			return updated;
		}

		auto inputFloat(const std::string& name, float& value, float min /*= -1.0f*/, float max /*= 1.0f*/) -> bool
		{
			bool updated = false;

			ImGui::TextUnformatted(name.c_str());
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			std::string id = "##" + name;
			if (ImGui::InputFloat(id.c_str(), &value, min, max))
				updated = true;

			ImGui::PopItemWidth();
			ImGui::NextColumn();
			return updated;
		}

		auto image(const Texture* texture, const glm::vec2& size) -> void
		{
			bool flipImage = true;//opengl is true 

			ImGui::Image(texture ? texture->getHandle() : nullptr, ImVec2(size.x, size.y), ImVec2(0.0f, flipImage ? 1.0f : 0.0f), ImVec2(1.0f, flipImage ? 0.0f : 1.0f));
		}

	};
};