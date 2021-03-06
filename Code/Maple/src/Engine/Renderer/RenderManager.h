//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <memory>
#include <vector>
#include "Engine/Timestep.h"
#include "Renderer.h"
#include "Engine/Core.h"

namespace Maple 
{	

	class Camera;
	class Scene;
	class GBuffer;
	class Texture;
	class ShadowRenderer;
	class OmniShadowRenderer;
	class PreProcessRenderer;
	class MAPLE_EXPORT RenderManager
	{
	public:
		RenderManager();
		auto init(uint32_t w,uint32_t h) -> void;
		auto beginScene(Scene* scene) -> void;
		auto onRender() -> void;
		auto onUpdate(const Timestep& step,Scene * scene) -> void;

		auto addRender(const std::shared_ptr<Renderer>& render) -> void;
		auto onImGui() -> void;

		auto onResize(uint32_t width, uint32_t height, bool debug = false) -> void;
		auto setRenderTarget(std::shared_ptr<Texture> texture, bool rebuildTexture,bool debug = false) -> void;
		inline auto& getGBuffer()  { return gbuffer; }

		inline auto& getRenderers() { return renders; }

		inline auto setShadowRenderer(ShadowRenderer* shadowRenderer) { this->shadowRenderer = shadowRenderer; }

		inline auto getShadowRenderer() const { return shadowRenderer; }

		inline auto setOmniShadowRenderer(OmniShadowRenderer* omniShadowRenderer) { this->omniShadowRenderer = omniShadowRenderer; }

		inline auto getOmniShadowRenderer() const { return omniShadowRenderer; }

		inline auto getPreProcessRenderer() const { return preProcessRenderer; }
		inline auto setPreProcessRenderer(PreProcessRenderer* preProcessRenderer) { this->preProcessRenderer = preProcessRenderer; }


		inline auto isEditor() const { return editor; }
		inline auto setEditor(bool val) { editor = val; }

	private:
		std::vector<std::shared_ptr<Renderer>> renders;
		std::shared_ptr<GBuffer> gbuffer;
		uint32_t width = 0;
		uint32_t height = 0;
		ShadowRenderer* shadowRenderer = nullptr;
		OmniShadowRenderer* omniShadowRenderer = nullptr;
		PreProcessRenderer* preProcessRenderer = nullptr;

		bool editor = false;
	};
};