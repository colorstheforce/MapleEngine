//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "RenderManager.h"
#include "Engine/GBuffer.h"
#include "Application.h"
#include "Engine/Camera.h"
#include "Window/NativeWindow.h"
#include "Scene/Scene.h"
#include "Engine/Profiler.h"

namespace Maple 
{
	RenderManager::RenderManager()
	{
		
	}

	auto RenderManager::init(uint32_t w, uint32_t h) -> void
	{
		width = w;
		height = h;
		gbuffer = std::make_shared<GBuffer>(w,h);
		for (auto & render : renders)
		{
			render->init(gbuffer);
		}
	}

	auto RenderManager::beginScene(Scene* scene) -> void
	{
		PROFILE_FUNCTION();
		for (auto& render : renders)
		{
			render->beginScene(scene);
		}
	}

	auto RenderManager::onRender() -> void
	{
		for (auto& render : renders)
		{
			render->renderScene();
		}
	}

	auto RenderManager::onUpdate(const Timestep& step, Scene* scene) -> void
	{
		scene->onUpdate(step);
	}

	auto RenderManager::addRender(const std::shared_ptr<Renderer>& render) -> void
	{
		render->setRenderManager(this);
		renders.emplace_back(render);
	}

	auto RenderManager::onImGui() -> void
	{
		PROFILE_FUNCTION();
		for (auto & renderer : renders)
		{
			renderer->onImGui();
		}
	}

	auto RenderManager::onResize(uint32_t width, uint32_t height, bool debug) -> void
	{
		this->width = width;
		this->height = height;
		gbuffer->resize(width, height);
		for (auto& render : renders)
		{
			render->onResize(width, height);
		}

		if(debug)
			Application::get()->getDebugRenderer().onResize(width, height);
	}

	auto RenderManager::setRenderTarget(std::shared_ptr<Texture>  texture,bool rebuildTexture, bool debug) -> void
	{
		for (auto& render : renders)
		{
			render->setRenderTarget(texture, rebuildTexture);
		}
		if (debug)
			Application::get()->getDebugRenderer().setRenderTarget(texture, rebuildTexture);
	}
};