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
#include "ImGui/ImGuiSystem.h"

#include "Engine/Timestep.h"
#include "Window/NativeWindow.h"
#include "Event/EventDispatcher.h"
#include "Others/Timer.h"
#include "Engine/Renderer/DebugRenderer.h"
#include "Thread/ThreadPool.h"
#include "Engine/TexturePool.h"
#include "Scene/System/SystemManager.h"
#include "Scripts/Lua/LuaVirtualMachine.h"


namespace Maple 
{
	class MonoVirtualMachine;

	enum class EditorState
	{
		Paused,
		Play,
		Next,
		Preview
	};

	class AppDelegate 
	{
	public:
		virtual auto onInit() -> void = 0;
		virtual auto onDestory() -> void = 0;
	};

	class DefaultDelegate final : public AppDelegate
	{
	public:
		virtual auto onInit() -> void override {};
		virtual auto onDestory() -> void override {};
	};


	class Application
	{
	public:
		Application(AppDelegate * appDelegate);
		virtual auto init() -> void;
		auto start() -> int32_t ;

		inline auto& getEventDispatcher() { return dispatcher; }
		inline auto& getWindow() { return window; }
		
		virtual auto onUpdate(const Timestep& delta) -> void;
		virtual auto onRender() -> void;
		virtual auto beginScene() -> void;
		virtual auto onImGui()->void;

		inline auto& getRenderDevice() { return rendererDevice; }
		inline auto& getSceneManager() { return sceneManager; }
		auto setSceneActive(bool active) -> void;
		inline auto& isSceneActive() const { return sceneActive; }

		inline auto& getEditorState() const { return state; }
		inline auto setEditorState(EditorState state) { this->state = state; }


		virtual auto onSceneCreated(Scene* scene) -> void;

		virtual auto onWindowResized(uint32_t w, uint32_t h) -> void;

		inline auto& getDebugRenderer() { return debugRender; }

		virtual auto onRenderDebug()->void;


		auto serialize() -> void;

		inline auto& getRenderManagers() const { return renderManagers; }

		auto postOnMainThread(const std::function<bool()>& mainCallback)->std::future<bool>;
		auto executeAll() -> void;
		inline auto& getThreadPool() { return threadPool; }
		template<class T>
		inline auto getAppDelegate() { return std::static_pointer_cast<T>(appDelegate); }
		inline auto& getTexturePool() { return texturePool; }
		inline auto& getLuaVirtualMachine() { return luaVm; }
		inline auto& getSystemManager() { return systemManager; }
		inline auto& getMonoVm() { return monoVm; }
	protected:

		std::unique_ptr<NativeWindow> window;
		std::unique_ptr<RenderDevice> rendererDevice;
		std::unique_ptr<SceneManager> sceneManager;
		std::unique_ptr<ThreadPool>	  threadPool;
		std::unique_ptr<TexturePool>  texturePool;
		std::unique_ptr<LuaVirtualMachine>  luaVm;
		std::shared_ptr<MonoVirtualMachine> monoVm;
		std::unique_ptr<SystemManager> systemManager;
		std::vector<std::unique_ptr<RenderManager>> renderManagers;

		std::shared_ptr<ImGuiSystem> imGuiManager;

		EventDispatcher dispatcher;
		Timer timer;
		uint64_t updates = 0;
		uint64_t frames = 0;
		float secondTimer = 0.0f;
		bool sceneActive = true;
		EditorState state = EditorState::Play;

		DebugRenderer debugRender;

		std::queue<std::pair<std::promise<bool>, std::function<bool()>>> executeQueue;
		std::mutex executeMutex;

		std::shared_ptr<AppDelegate> appDelegate;
	};
};

extern Maple::Application * app;
