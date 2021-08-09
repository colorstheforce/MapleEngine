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
#include "Scene/Component/Light.h"

namespace Maple 
{
#define SHADOWMAP_MAX 4

	class UniformBuffer;
	class Texture2D;
	class TextureDepthArray;
	class Pipeline;
	class Transform;
	class TextureDepth;

	class ShadowRenderer : public Renderer
	{
	public:
		ShadowRenderer(const std::shared_ptr<TextureDepthArray> & texture = nullptr, uint32_t size = 4096, uint32_t numMaps = 4);
		~ShadowRenderer();

		auto init(const std::shared_ptr<GBuffer>& buffer) -> void override;
		auto begin() -> void override;
		auto present() -> void override;
		auto end() -> void override;
		auto submit(const RenderCommand& cmd) -> void override;
		auto submit(const RenderCommand& cmd,int32_t i) -> void;
		auto renderScene() -> void override;
		auto beginScene(Scene* scene) -> void override;
		auto onResize(uint32_t width, uint32_t height) -> void override {};

		inline auto getShadowTexture() const { return shadowTexture; }
		inline auto getShadowCascadeTransform() const { return systemUniformBuffer.projView; }
		inline auto& getLightViewMatrix() const { return lightViewMatrix; }
		inline auto& getBias() const { return initialBias; }
		inline auto& getLightSize() const { return lightSize; }
		inline auto& getSplitDepth() const { return splitDepth; }

		auto onImGui() -> void override;

	private:
		auto createFrameBuffers() -> void;
		auto createRenderPass() -> void;
		auto createPipeline() -> void;
		auto createUniformBuffer() -> void;

		auto updateCascades(const Scene* scene, Camera* camera, Transform * transform, const Light* light) -> void;


		struct UniformBufferObject
		{
			glm::mat4 projView[SHADOWMAP_MAX];
		};

		UniformBufferObject systemUniformBuffer;

		std::shared_ptr<DescriptorSet> descriptorSet;
		std::shared_ptr<TextureDepthArray> shadowTexture;
		std::shared_ptr<Pipeline> pipeline;
		std::shared_ptr<UniformBuffer> uniformBuffer;


		//bias for cast shadow
		float initialBias = 0.0005f;
		float cascadeSplitLambda = 0.95;
		float splitDepth[SHADOWMAP_MAX];

		float lightSize = 0.1;

		uint32_t shadowMapSize = 0;
		uint32_t bufferId = 0;
		uint32_t numMaps = 0;
		bool shadowMapsInvalidated = true;
		bool hasLight = false;
		int32_t cascadeIndex = 0;
		std::array<std::shared_ptr<FrameBuffer>, SHADOWMAP_MAX> shadowFrameBuffers;
		std::array<std::vector<RenderCommand>, SHADOWMAP_MAX> cascadeCommandQueue;
		glm::mat4 lightViewMatrix;

		std::shared_ptr<FrameBuffer> omniFrameBuffer;

	};
};