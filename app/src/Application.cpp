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
#include "Terrain/TerrainBuilder.h"
#include "Devices/Input.h"
#include "ImGui/ImGuiManager.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>

#include "Engine/Vulkan/VulkanContext.h"


Maple::Application* app;

namespace Maple 
{
	Application::Application()
	{
		window =			NativeWindow::newInstance(WindowInitData{ 1280,720,false,"SampleApp" });
		sceneManager =		std::make_unique<SceneManager>();
		rendererDevice =			std::make_unique<VkRenderDevice>(window->getWidth(), window->getHeight());
		imGuiManager =		std::make_unique<ImGuiManager>(false);
		renderManager =		std::make_unique<RenderManager>();
	}

	auto Application::init() -> void
	{
		Input::create();
		window->init();
		timer.start();
		rendererDevice->init();

		//off-screen
		renderManager->addRender(std::make_unique<PreProcessRenderer>());
		renderManager->addRender(std::make_unique<ShadowRenderer>());

	//	renderManager->addRender(std::make_unique<OmniShadowRenderer>());
		renderManager->addRender(std::make_unique<DeferredRenderer>(window->getWidth(), window->getHeight()));
		renderManager->addRender(std::make_unique<SkyboxRenderer>(window->getWidth(), window->getHeight()));
		renderManager->addRender(std::make_unique<GridRenderer>(window->getWidth(), window->getHeight()));
		renderManager->addRender(std::make_unique<Renderer2D>(window->getWidth(), window->getHeight()));

		debugRender.init(window->getWidth(), window->getHeight());
		renderManager->init(window->getWidth(), window->getHeight());

		imGuiManager->init();
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
				auto tStart = std::chrono::high_resolution_clock::now();
				
				onUpdate(timestep);
				onRender();

				imGuiManager->onRender(sceneManager->getCurrentScene());
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
		return 0;
	}


	//update all things
	auto Application::onUpdate(const Timestep& delta) -> void
	{
		onImGui();
		imGuiManager->onUpdate(delta);
		window->onUpdate();
		dispatcher.dispatchEvents();
		renderManager->onUpdate(delta, sceneManager->getCurrentScene());
	}

	auto Application::onRender() -> void
	{
		renderManager->beginScene(sceneManager->getCurrentScene());
		debugRender.beginScene(sceneManager->getCurrentScene());
		rendererDevice->begin();
		renderManager->onRender();
		onRenderDebug();
		debugRender.renderScene();
	}

	auto Application::onImGui() ->void
	{
		renderManager->onImGui();
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
		renderManager->onResize(w, h);

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

};
