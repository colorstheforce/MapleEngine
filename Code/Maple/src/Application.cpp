//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "Application.h"
#include "Window/WindowWin.h"
#include "Others/Console.h"
#include "FileSystem/MeshLoader.h"
#include "Engine/Timestep.h"
#include "Engine/Camera.h"
#include "Engine/Renderer/VkRenderDevice.h"
#include "Engine/Renderer/RenderManager.h"
#include "Engine/Renderer/ForwardRenderer.h"
#include "Engine/Renderer/DeferredRenderer.h"
#include "Engine/Renderer/SkyboxRenderer.h"
#include "Engine/Renderer/ShadowRenderer.h"
#include "Engine/Renderer/GridRenderer.h"
#include "Engine/Renderer/PreProcessRenderer.h"
#include "Engine/Renderer/OmniShadowRenderer.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Terrain.h"

#include "Scripts/Lua/LuaSystem.h"
#include "Scripts/Mono/MonoSystem.h"

#include "Terrain/TerrainBuilder.h"
#include "Devices/Input.h"
#include "ImGui/ImGuiSystem.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>

#include "Engine/Vulkan/VulkanContext.h"
#include "Scripts/Mono/MonoVirtualMachine.h"

Maple::Application* app;

namespace Maple 
{
	Application::Application(AppDelegate* app)
	{
		appDelegate			= std::shared_ptr<AppDelegate>(app);
		window				= NativeWindow::newInstance(WindowInitData{ 1280,720,false,"Maple-Engine" });
		sceneManager		= std::make_unique<SceneManager>();
		rendererDevice		= std::make_unique<VkRenderDevice>(window->getWidth(), window->getHeight());
		imGuiManager		= std::make_unique<ImGuiSystem>(false);
		threadPool			= std::make_unique<ThreadPool>(4);
		texturePool			= std::make_unique<TexturePool>();
		luaVm				= std::make_unique<LuaVirtualMachine>();
		monoVm				= std::make_shared<MonoVirtualMachine>();
		systemManager		= std::make_unique<SystemManager>();
	}

	auto Application::init() -> void
	{
		Input::create();
		window->init();
		timer.start();
		rendererDevice->init();
		luaVm->init();
		monoVm->init();

		auto render = std::make_unique<RenderManager>();
		render->addRender(std::make_unique<PreProcessRenderer>());
		render->addRender(std::make_unique<ShadowRenderer>()) ;
		render->addRender(std::make_unique<DeferredRenderer>(window->getWidth(), window->getHeight()));
		render->addRender(std::make_unique<SkyboxRenderer>(window->getWidth(), window->getHeight()));
		render->addRender(std::make_unique<Renderer2D>(window->getWidth(), window->getHeight()));
		debugRender.  init(window->getWidth(), window->getHeight());
		render->	  init(window->getWidth(), window->getHeight());
		renderManagers.emplace_back(std::move(render));
		appDelegate->onInit();
		

		systemManager->addSystem<LuaSystem>()->onInit();
		systemManager->addSystem<MonoSystem>()->onInit();
		imGuiManager = systemManager->addSystem<ImGuiSystem>(false);
		imGuiManager->onInit();
	}

	auto Application::start() -> int32_t
	{
		double lastFrameTime = 0;
		init();
	
		while (1) 
		{
			Timestep timestep = timer.stop() / 1000000.f;
			ImGuiIO& io = ImGui::GetIO();
			io.DeltaTime = timestep.getMilliseconds();
			Input::getInput()->resetPressed();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			{
				sceneManager->apply();
				//auto tStart = std::chrono::high_resolution_clock::now();
				executeAll();
				onUpdate(timestep);
				onRender();

				rendererDevice->end();
				rendererDevice->present();//present all data
			}
		
			lastFrameTime += timestep;
			if (lastFrameTime - secondTimer > 1.0f) 
			{
				secondTimer += 1.0f;
				frames = 0;
				updates = 0;
				//tick later
				LOGI("FPS : {0}, Delta time : {1}", io.Framerate,io.DeltaTime * 1000);
			}
		}
		appDelegate->onDestory();
		return 0;
	}


	//update all things
	auto Application::onUpdate(const Timestep& delta) -> void
	{
		onImGui();
		systemManager->onUpdate(delta, sceneManager->getCurrentScene());
		window->onUpdate();
		dispatcher.dispatchEvents();
		for (auto& r : renderManagers)
		{
			r->onUpdate(delta, sceneManager->getCurrentScene());
		}
	}

	auto Application::onRender() -> void
	{
		for (auto& r : renderManagers)
		{
			sceneManager->getCurrentScene()->setGameView(!r->isEditor());
			r->beginScene(sceneManager->getCurrentScene());
			if (r->isEditor()) {
				debugRender.beginScene(sceneManager->getCurrentScene());
			}
		}

		rendererDevice->begin();

		for (auto& r : renderManagers)
		{
			r->onRender();
		}
	
		onRenderDebug();
		debugRender.renderScene();


		imGuiManager->onRender(sceneManager->getCurrentScene());
	}

	auto Application::beginScene() -> void
	{

	}

	auto Application::onImGui() ->void
	{
		for (auto& r : renderManagers)
		{
			r->onImGui();
		}
	}

	auto Application::setSceneActive(bool active) -> void
	{
		sceneActive = active;
	}

	auto Application::onSceneCreated(Scene* scene) -> void
	{

	}

	auto Application::onWindowResized(uint32_t w, uint32_t h) -> void
	{
		VulkanContext::get()->waiteIdle();

		if (w == 0 || h == 0)
		{
			return ;
		}

		rendererDevice->onResize(w, h);
		imGuiManager->onResize(w, h);
		for (auto & r : renderManagers)
		{
			r->onResize(w, h);
		}

		VulkanContext::get()->waiteIdle();
	}

	auto Application::onRenderDebug() ->void
	{
	
	}

	auto Application::serialize() -> void
	{
		if(sceneManager->getCurrentScene() != nullptr){
			sceneManager->getCurrentScene()->saveTo();
			window->setTitle(sceneManager->getCurrentScene()->getName());
		}
	}

	auto Application::postOnMainThread(const std::function<bool()>& mainCallback) ->std::future<bool>
	{
		std::promise<bool> promise;
		std::future<bool> future = promise.get_future();

		std::lock_guard<std::mutex> locker(executeMutex);
		executeQueue.emplace(std::move(promise), mainCallback);
		return future;
	}

	auto Application::executeAll() -> void
	{
		std::pair<std::promise<bool>, std::function<bool(void)>> func;
		for (;;) {
			{
				std::lock_guard<std::mutex> lock(executeMutex);
				if (executeQueue.empty())
					break;
				func = std::move(executeQueue.front());
				executeQueue.pop();
			}
			if (func.second) {
				func.first.set_value(func.second());
			}
		}
	}

};
