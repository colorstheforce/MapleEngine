//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 

////////////////////////////////////////////////////////////////////////////// 

#include "ImGuiSystem.h"
#include "VkImGUIRenderer.h"
#include "Application.h"
#include "Window/WindowWin.h"

#include "ImGuiHelpers.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <IconsMaterialDesignIcons.h>
#include <MaterialDesign.inl>


namespace Maple
{
    ImGuiSystem::ImGuiSystem(bool clearScreen)
        :clearScreen(clearScreen)
    {

    }
    ImGuiSystem::~ImGuiSystem()
    {
    }
    auto ImGuiSystem::onInit() -> void
    {

		app->getEventDispatcher().addEventHandler(&handler);

		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		
		ImGuiIO& io = ImGui::GetIO();
		
		io.DisplaySize = ImVec2(static_cast<float>(app->getWindow()->getWidth()),
            static_cast<float>(app->getWindow()->getHeight()));

		addIcon();


		imguiRender = std::make_unique<VkImGUIRenderer>(
            app->getWindow()->getWidth(), 
            app->getWindow()->getHeight(),
            clearScreen);


		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	
		imguiRender->init();
		ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)app->getWindow()->getNativeInterface(), true);
		/*io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.ConfigWindowsMoveFromTitleBarOnly = true;*/

		setTheme();
    }

    auto ImGuiSystem::onUpdate(float dt, Scene* scene) -> void
    {
        ImGui::Render();
    }

    auto ImGuiSystem::onRender(Scene* scene) -> void
    {
        imguiRender->render(nullptr);
    }

	auto ImGuiSystem::addIcon() -> void
	{
		ImGuiIO& io = ImGui::GetIO();
		static const ImWchar icons_ranges[] = { ICON_MIN_MDI, ICON_MAX_MDI, 0 };
		ImFontConfig icons_config;
		io.Fonts->AddFontDefault();
		// merge in icons from Font Awesome
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		icons_config.GlyphOffset.y = 1.0f;
		icons_config.OversampleH = icons_config.OversampleV = 1;
		icons_config.PixelSnapH = true;
		icons_config.SizePixels = 18.f;
		io.Fonts->AddFontFromMemoryCompressedTTF(MaterialDesign_compressed_data, MaterialDesign_compressed_size, 16, &icons_config, icons_ranges);
	}

	auto ImGuiSystem::onResize(uint32_t w, uint32_t h) -> void
	{
		imguiRender->onResize(w, h);
	}

	auto ImGuiSystem::setTheme() -> void
	{
		

	}

};