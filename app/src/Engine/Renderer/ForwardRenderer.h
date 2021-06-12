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
	class VulkanDescriptorSet;
	class VulkanUniformBuffer;
	class Texture2D;
	class VulkanFrameBuffer;
	class Camera;

	class ForwardRenderer : public Renderer
	{
	public:
		ForwardRenderer() = default;
		ForwardRenderer(uint32_t width, uint32_t height, bool depthTest = true);
		~ForwardRenderer();

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


		struct UniformBufferObject {
			alignas(16) glm::mat4 model;
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
			uint32_t width;
			uint32_t height;
			int32_t calInVertex;
		};

		auto updateUniformBuffer(const RenderCommand& cmd) -> void;
		auto createFrameBuffers() -> void;

	
		std::vector<std::shared_ptr<VulkanUniformBuffer>> uniformBuffers;
		std::vector<std::shared_ptr<VulkanUniformBuffer>> lightUniformBuffers;
		std::vector<std::shared_ptr<VulkanUniformBuffer>> materialUniformBuffers;

		UniformBufferObject systemVsUniformBuffer;
		uint32_t bufferId = 0;
	
		std::unique_ptr<VulkanDescriptorSet> descriptorSet;

		std::shared_ptr<Texture2D> defaultTexture;

		std::shared_ptr<Camera> camera;
	};
};