//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 
////////////////////////////////////////////////////////////////////////////// 

#pragma once
#include "Engine/Timestep.h"
#include "Event/EventHandler.h"
#include <memory>

namespace Maple
{
	class VkImGUIRenderer ;
	class Scene;
	class ImGuiManager
	{
	public:
		ImGuiManager(bool clearScreen = false);
		~ImGuiManager();

		auto init() -> void;
		auto onUpdate(const Timestep & delta) -> void ;
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