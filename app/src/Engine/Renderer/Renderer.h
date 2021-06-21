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
namespace Maple 
{
	class FrameBuffer;
	class RenderPass;
	class Shader;
	class Pipeline;

	class Texture;
	class Camera;
	class Scene;
	class GBuffer;
	class CommandBuffer;
	class DescriptorSet;


	class Renderer
	{
	public:
		virtual ~Renderer() = default;
		virtual auto init(const std::shared_ptr<GBuffer>& buffer) -> void = 0;
		virtual auto begin() -> void = 0;
		virtual auto present() -> void = 0;
		virtual auto end() -> void = 0;
		virtual auto submit(const RenderCommand& cmd) -> void = 0;
		virtual auto renderScene() -> void = 0;
		virtual auto beginScene(Scene* scene) -> void = 0;
		virtual auto onResize(uint32_t width, uint32_t height) -> void {};
		virtual auto onImGui() -> void {};
		virtual auto setRenderTarget(std::shared_ptr <Texture> texture, bool rebuildFramebuffer = true) -> void { renderTexture = texture; }


		auto bindDescriptorSets(Pipeline* pipeline, CommandBuffer* cmdBuffer, uint32_t dynamicOffset, const std::vector<std::shared_ptr<DescriptorSet>>& descriptorSets) -> void;
		auto drawIndexed(CommandBuffer* commandBuffer, DrawType type, uint32_t count, uint32_t start = 0) -> void;


		auto getCommandBuffer()->CommandBuffer*;

	protected:
		uint32_t width;
		uint32_t height;

		std::shared_ptr<RenderPass> renderPass;
		std::shared_ptr<Shader> shader;
		std::shared_ptr<Pipeline> pipeline;
		std::shared_ptr<Texture> depthTexture;
		std::shared_ptr<Texture> renderTexture;
		std::shared_ptr<GBuffer> gbuffer;
		std::vector<RenderCommand> commandQueue;

		std::vector <std::shared_ptr<FrameBuffer>> frameBuffers;


		bool depthTest = true;
	};
};