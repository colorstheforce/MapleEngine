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

namespace Maple
{
	class UniformBuffer;

	class GridRenderer : public Renderer
	{
	public:
		GridRenderer() = default;
		GridRenderer(uint32_t width, uint32_t height);
		~GridRenderer();

		auto init(const std::shared_ptr<GBuffer>& buffer) -> void override;
		auto begin() -> void override;
		auto present() -> void override;
		auto end() -> void override;
		auto submit(const RenderCommand& cmd) -> void override;
		auto renderScene() -> void override;
		auto beginScene(Scene* scene) -> void override;
		auto onResize(uint32_t width, uint32_t height) -> void override;
		auto setRenderTarget(std::shared_ptr <Texture>, bool rebuildFramebuffer = true) -> void override;

	private:
		
		struct UniformBufferObject
		{
			glm::mat4 projView;
		};

		struct UniformBufferObjectFrag
		{
			glm::vec4 cameraPos;
			float scale;
			float res;
			float maxDistance;
			float p1;
		};

	
		auto createFrameBuffers() -> void;
		auto createRenderPass() -> void;
		auto createPipeline() -> void;
		auto createUniformBuffer() -> void;

		std::shared_ptr<UniformBuffer> uniformBuffer;
		std::shared_ptr<UniformBuffer> uniformBufferFrag;

		UniformBufferObject systemUniformBuffer;
		UniformBufferObjectFrag systemUniformBufferFrag;

		uint32_t currentBufferID = 0;
		std::shared_ptr<Mesh> quad;

		float gridRes = 1.4f;
		float gridSize = 500.0f;
		float maxDistance = 600.0f;

	};
};