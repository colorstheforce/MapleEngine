

#include "LoadingDialog.h"
#include "imgui_widgets_user.h"

namespace Maple
{
	auto LoadingDialog::show(const std::string& name) -> void
	{
		this->name = name;
		active = true;
	}

	auto LoadingDialog::close() -> void
	{
		active = false;
		ImGui::CloseCurrentPopup();
	}

 	auto LoadingDialog::onImGui() -> void
	{
		if (active) {
			ImGui::OpenPopup(name.c_str());
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,5);

			if (ImGui::BeginPopupModal(name.c_str(),nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
				const ImU32 bg = ImGui::GetColorU32(ImGuiCol_Button);

				ImGui::Spinner("##spinner", 15, 6, col);
				ImGui::BufferingBar("##buffer_bar", 0.7f, ImVec2(400, 6), bg, col);

			
				ImGui::EndPopup();
			}
			ImGui::PopStyleVar();
		}
	}

};