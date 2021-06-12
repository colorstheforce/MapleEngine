//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <memory>
#include <list>

#include "Engine/Renderer/RenderDevice.h"
#include "ImGui/VkImGUIRenderer.h"
#include "Engine/Renderer/RenderManager.h"
#include "Scene/SceneManager.h"
#include "ImGui/ImGuiManager.h"

#include "Engine/Timestep.h"
#include "Window/NativeWindow.h"
#include "Event/EventDispatcher.h"
#include "Others/Timer.h"
#include "Engine/Renderer/DebugRenderer.h"

namespace Maple 
{
	enum class EditorState
	{
		Paused,
		Play,
		Next,
		Preview
	};

	class Application
	{
	public:
		Application();
		virtual auto init() -> void;
		auto start() -> int32_t ;

		inline auto& getEventDispatcher() { return dispatcher; }
		inline auto& getWindow() { return window; }
		
		virtual auto onUpdate(const Timestep& delta) -> void;
		virtual auto onRender() -> void;
		virtual auto onImGui()->void;

		inline auto& getRenderDevice() { return rendererDevice; }
		inline auto& getSceneManager() { return sceneManager; }
		inline auto& getRenderManager() { return renderManager; }
		auto setSceneActive(bool active) -> void;
		inline auto& isSceneActive() const { return sceneActive; }

		inline auto& getEditorState() const { return state; }
		inline auto setEditorState(EditorState state) { this->state = state; }


		virtual auto onSceneCreated(Scene* scene) -> void;

		virtual auto onWindowResized(uint32_t w, uint32_t h) -> void;

		inline auto& getDebugRenderer() { return debugRender; }

		virtual auto onRenderDebug()->void;


		auto serialize() -> void;

	protected:
		std::unique_ptr<NativeWindow> window;
		std::unique_ptr<RenderDevice> rendererDevice;
		std::unique_ptr<ImGuiManager> imGuiManager;
		std::unique_ptr<SceneManager> sceneManager;
		std::unique_ptr<RenderManager> renderManager;


		EventDispatcher dispatcher;
		Timer timer;
		uint64_t updates = 0;
		uint64_t frames = 0;
		float secondTimer = 0.0f;
		bool sceneActive = true;
		EditorState state = EditorState::Play;

		DebugRenderer debugRender;

	public:
	

public:



};
};

extern Maple::Application * app;
