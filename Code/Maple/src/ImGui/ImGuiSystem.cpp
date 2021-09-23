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
#include <RobotoRegular.inl>
#include <MaterialDesign.inl>
#include "Engine/Profiler.h"

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
		PROFILE_FUNCTION();
		Application::get()->getEventDispatcher().addEventHandler(&handler);

		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		
		ImGuiIO& io = ImGui::GetIO();
		
		io.DisplaySize = ImVec2(
			static_cast<float>(Application::get()->getWindow()->getWidth()),
			static_cast<float>(Application::get()->getWindow()->getHeight())
		);

		ImGui::GetStyle().ScaleAllSizes(Application::get()->getWindow()->getScale());

		addIcon();


		imguiRender = std::make_unique<VkImGUIRenderer>(
            Application::get()->getWindow()->getWidth(), 
            Application::get()->getWindow()->getHeight(),
            clearScreen);


		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	
		imguiRender->init();
		ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)Application::get()->getWindow()->getNativeInterface(), true);
		/*io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.ConfigWindowsMoveFromTitleBarOnly = true;*/

		setTheme();
    }

    auto ImGuiSystem::onUpdate(float dt, Scene* scene) -> void
    {
		PROFILE_FUNCTION();
        ImGui::Render();
    }

    auto ImGuiSystem::onRender(Scene* scene) -> void
    {
		PROFILE_FUNCTION();
        imguiRender->render(nullptr);
    }

	auto ImGuiSystem::addIcon() -> void
	{
		PROFILE_FUNCTION();
		ImGuiIO& io = ImGui::GetIO();
		static const ImWchar icons_ranges[] = { ICON_MIN_MDI, ICON_MAX_MDI, 0 };
		//io.Fonts->AddFontFromFileTTF("fonts/simsun.ttf", 16.f, NULL, io.Fonts->GetGlyphRangesChineseFull());
	//	io.Fonts->AddFontDefault();
	

		ImFontConfig iconsConfig;
		iconsConfig.MergeMode = false;
		iconsConfig.PixelSnapH = true;
		iconsConfig.OversampleH = iconsConfig.OversampleV = 1;
		iconsConfig.GlyphMinAdvanceX = 4.0f;
		iconsConfig.SizePixels = 12.0f;

		static const ImWchar ranges[] = {
			0x0020,
			0x00FF,
			0x0400,
			0x044F,
			0,
		};

		io.Fonts->AddFontFromMemoryCompressedTTF(
			RobotoRegular_compressed_data, 
			RobotoRegular_compressed_size, 16.f * Application::get()->getWindow()->getScale(),
			&iconsConfig,
			ranges);




		iconsConfig.MergeMode = true;
		iconsConfig.PixelSnapH = true;
		iconsConfig.GlyphOffset.y = 1.0f;
		iconsConfig.OversampleH = iconsConfig.OversampleV = 1;
		iconsConfig.PixelSnapH = true;
		iconsConfig.SizePixels = 18.f;
		io.Fonts->AddFontFromMemoryCompressedTTF(
			MaterialDesign_compressed_data, 
			MaterialDesign_compressed_size, 16 * Application::get()->getWindow()->getScale(),
			&iconsConfig, icons_ranges);
	}

	auto ImGuiSystem::onResize(uint32_t w, uint32_t h) -> void
	{
		PROFILE_FUNCTION();
		imguiRender->onResize(w, h);
	}

	auto ImGuiSystem::setTheme() -> void
	{
		

	}

};