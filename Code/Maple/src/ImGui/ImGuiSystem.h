//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 
////////////////////////////////////////////////////////////////////////////// 

#pragma once
#include "Engine/Timestep.h"
#include "Event/EventHandler.h"
#include "Scene/System/ISystem.h"
#include <memory>

namespace Maple
{
	class VkImGUIRenderer ;
	class Scene;

	class ImGuiSystem : public ISystem
	{
	public:
		ImGuiSystem(bool clearScreen = false);
		~ImGuiSystem();
		
		auto onInit() -> void override;
		auto onUpdate(float dt, Scene* scene) -> void override;
		auto onImGui() -> void override {};

		auto onRender(Scene * scene) -> void;
		auto addIcon() -> void;
		auto onResize(uint32_t w, uint32_t h) -> void;
		auto setTheme() -> void;
	private:

		bool clearScreen = false;
		std::unique_ptr<VkImGUIRenderer> imguiRender;
		EventHandler handler;
	};
}