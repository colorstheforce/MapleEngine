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
	class Texture;
	class UniformBuffer;


	class SkyboxRenderer : public Renderer
	{
	public:
		SkyboxRenderer(uint32_t width, uint32_t height);
		~SkyboxRenderer();

		auto init(const std::shared_ptr<GBuffer>& buffer) -> void override;
		auto begin() -> void override;
		auto present() -> void override;
		auto end() -> void override;
		auto submit(const RenderCommand& cmd) -> void override {};
		auto renderScene() -> void override;
		auto beginScene(Scene* scene) -> void override;
		auto onResize(uint32_t width, uint32_t height) -> void override;
		auto setCubeMap(const std::shared_ptr<Texture>& cubeMap) -> void;
		auto setRenderTarget(std::shared_ptr <Texture> texture, bool rebuildFramebuffer /*= true*/) -> void override;
	private:

	
		struct UniformBufferObject
		{
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};			   

		auto createPipeline() -> void;
		auto updateUniform() -> void;
		auto createFrameBuffer() -> void;
		std::shared_ptr<Mesh> skybox;
		std::shared_ptr<Texture> cubeMap;
		std::shared_ptr<UniformBuffer> uniformBuffer;

		UniformBufferObject uniformBufferObj;
	};
};