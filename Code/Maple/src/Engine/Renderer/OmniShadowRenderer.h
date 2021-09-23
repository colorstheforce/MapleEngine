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

	class UniformBuffer;
	class Texture2D;
	class TextureDepthArray;
	class Pipeline;
	class Transform;
	class TextureCube;

	class MAPLE_EXPORT OmniShadowRenderer : public Renderer
	{
	public:
		OmniShadowRenderer(const std::shared_ptr<TextureCube> & texture = nullptr, uint32_t size = 1024);
		~OmniShadowRenderer();

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
		auto onImGui() -> void override;
		inline auto& getLightViews() const { return lightViews; }

	private:
		auto createFrameBuffers() -> void;
		auto createRenderPass() -> void;
		auto createPipeline() -> void;
		auto createUniformBuffer() -> void;


		struct UniformBufferObject
		{
			glm::mat4 projView;
			glm::vec3 lightPos;
			float radius;
		};

		UniformBufferObject systemUniformBuffer;

		std::shared_ptr<DescriptorSet> descriptorSet;
		std::shared_ptr<TextureCube> shadowTexture;
		std::shared_ptr<Pipeline> pipeline;
		std::shared_ptr<UniformBuffer> uniformBuffer;

		bool hasLight = false;
		std::shared_ptr<FrameBuffer>  shadowFrameBuffer;
		std::array<std::vector<RenderCommand>, 6> shadowCommandQueue;
		std::array<glm::mat4, 6> lightViews;
		uint32_t shadowMapSize = 0;
		uint32_t commandQueueId = 0;

		std::shared_ptr<Texture2D> colorAttachment;

	};
};