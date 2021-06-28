//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "Engine/Vertex.h"
#include "Engine/Quad2D.h"
#include "RenderParam.h"
#include "Renderer.h"

namespace Maple 
{
	class UniformBuffer;
	class Texture2D;
	class VertexBuffer;
	class IndexBuffer;
	class DescriptorSet;

	struct Config2D
	{
		uint32_t maxQuads = 10000;
		uint32_t quadsSize = sizeof(Vertex2D) * 4;
		uint32_t bufferSize = 10000 * sizeof(Vertex2D) * 4;
		uint32_t indiciesSize = 10000 * 6;
		uint32_t maxTextures = 16;
		uint32_t maxBatchDrawCalls = 10;

		auto setMaxQuads(uint32_t quads) -> void
		{
			maxQuads = quads;
			bufferSize =   maxQuads * sizeof(Vertex2D) * 4;
			indiciesSize = maxQuads * 6;
		}
	};

	struct Triangle
	{
		glm::vec3 pos1;
		glm::vec3 pos2;
		glm::vec3 pos3;
		glm::vec4 color;
	};

	struct Command2D 
	{
		Quad2D* quad;
		glm::mat4 transform;
	};

	class Renderer2D : public Renderer
	{
	public:
		Renderer2D() = default;
		//if use renderer2d in 3D model, depth should be enabled, otherwise.
		Renderer2D(uint32_t width, uint32_t height,bool enableDepth = true);
		~Renderer2D();

		auto init(const std::shared_ptr<GBuffer>& buffer) -> void override;
		auto begin() -> void override;
		auto present() -> void override;
		auto submitQuad() -> void;
		auto end() -> void override;
		auto submit(const RenderCommand& cmd) -> void override;
		auto renderScene() -> void override;
		auto beginScene(Scene* scene) -> void override;
		auto onResize(uint32_t width, uint32_t height) -> void override;
		auto setRenderTarget(std::shared_ptr <Texture>, bool rebuildFramebuffer = true) -> void override;
		auto submit(Quad2D * quad, const glm::mat4 & transform) -> void;
		

	private:

		auto submitTexture(const std::shared_ptr<Texture>& texture)->int32_t;
		auto updateDesciptorSet() -> void;
		struct UniformBufferObject {
			glm::mat4 projView;
		};

		auto flush() -> void;
		auto createFrameBuffers() -> void;
		std::vector<Command2D> commands;
		std::shared_ptr<UniformBuffer> uniformBuffer;
		UniformBufferObject systemVsUniformBuffer;
		std::vector<std::shared_ptr<CommandBuffer>> secondaryCommandBuffers;
		std::vector<std::shared_ptr<VertexBuffer>> vertexBuffers;
		std::shared_ptr<IndexBuffer> indexBuffer;
		std::vector<std::shared_ptr<Texture>> textures;

		std::shared_ptr<DescriptorSet> descriptorSet;

		Vertex2D * buffer = nullptr;
		Config2D config;
		uint32_t indexCount = 0;
		uint32_t textureCount = 0;
		uint32_t batchDrawCallIndex = 0;
		bool enableDepth = true;

		std::vector<uint32_t> indicesBatches;


	};
};