//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "RenderParam.h"
#include "Renderer.h"
#include "Event/EventHandler.h"
#include "Scene/Component/Light.h"

namespace Maple 
{
#define MAX_LIGHTS 32
#define MAX_SHADOWMAPS 4

	class TextureCube;
	class DescriptorSet;
	class UniformBuffer;
	class Texture2D;
	class FrameBuffer;
	class DeferredOffScreenRenderer;

	class DeferredRenderer : public Renderer
	{
	public:
		DeferredRenderer() = default;
		DeferredRenderer(uint32_t width, uint32_t height, bool depthTest = true);
		~DeferredRenderer();

		auto init(const std::shared_ptr<GBuffer>& buffer) -> void override;
		auto begin() -> void override;
		auto present() -> void override;
		auto end() -> void override;
		auto submit(const RenderCommand& cmd) -> void override;
		auto renderScene() -> void override;
		auto beginScene(Scene* scene) -> void override;
		auto onResize(uint32_t width, uint32_t height) -> void override;
		auto setRenderTarget(std::shared_ptr <Texture>, bool rebuildFramebuffer = true) -> void override;
		auto createLightBuffer() -> void;

		auto onImGui() -> void override;

	private:
		auto updateScreenDescriptorSet() -> void;

		auto submitLight(Scene* scene) -> void;


		struct UniformBufferObject {
			LightData lights[MAX_LIGHTS]; //32 Bytes * 32
			glm::mat4 viewPos;//16
			glm::mat4 lightProjView[4];//cascade proj view
			glm::mat4 lightView;
			glm::vec4 cameraPos;

			float splitDepth[4];

			//16 bytes
			int32_t lightCount; //4
			int32_t type;
			int32_t colorCascade;
			int32_t displayCascadeLevel;

			//16 bytes
			float bias;
			float lightSize;
			float bias2;
			float bias3;

			int32_t prefilterLODLevel;
			int32_t padding1;
			int32_t padding2;
			int32_t padding3;

		};

		auto createFrameBuffers() -> void;
	
		std::vector<std::shared_ptr<UniformBuffer>> uniformBuffers;
		std::vector<std::shared_ptr<UniformBuffer>> materialUniformBuffers;

		UniformBufferObject systemVsUniformBuffer = {};

	
		std::shared_ptr<DescriptorSet> descriptorSet;
		std::shared_ptr<UniformBuffer> lightUniformBuffer;

		std::shared_ptr<Texture2D> defaultTexture;
		std::shared_ptr <Mesh> screenQuad;

		DeferredOffScreenRenderer * deferredOffScreenRenderer = nullptr;

		EventHandler eventHandler;

		std::shared_ptr<TextureCube>  environmentMap;
		std::shared_ptr<TextureCube>  irradianceMap;

		std::shared_ptr<Texture2D> preIntegratedFG;
	};
};