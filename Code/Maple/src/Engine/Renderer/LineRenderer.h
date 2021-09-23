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
	class VertexBuffer;
	class IndexBuffer;


	struct LineVertex
	{
		glm::vec3 vertex;
		glm::vec4 color;

		bool operator==(const LineVertex& other) const
		{
			return vertex == other.vertex && color == other.color;
		}
	};

	struct Line
	{
		glm::vec3 start;
		glm::vec3 end;
		glm::vec4 color;
	};


	class MAPLE_EXPORT LineRenderer : public Renderer
	{
	public:
		LineRenderer() = default;
		LineRenderer(uint32_t width, uint32_t height, bool depthTest = true);
		~LineRenderer();

		auto init(const std::shared_ptr<GBuffer>& buffer) -> void override;
		auto begin() -> void override;
		auto present() -> void override;
		auto end() -> void override;
		auto submit(const RenderCommand& cmd) -> void override;
		auto renderScene() -> void override;
		auto beginScene(Scene* scene) -> void override;
		auto onResize(uint32_t width, uint32_t height) -> void override;
		auto setRenderTarget(std::shared_ptr <Texture>, bool rebuildFramebuffer = true) -> void override;
		auto renderInternal() -> void;
		auto clear() -> void;

		auto submit(const glm::vec3& p1, const glm::vec3& p2, const  glm::vec4& color) -> void;


	private:

		auto submitLines() -> void;
		auto flush() -> void;

		struct UniformBufferObject
		{
			glm::mat4 projView;
		};
	
		auto createFrameBuffers() -> void;
		auto createRenderPass() -> void;
		auto createPipeline() -> void;
		auto createUniformBuffer() -> void;

		std::shared_ptr<UniformBuffer> uniformBuffer;

		UniformBufferObject systemUniformBuffer;

		


		std::vector<std::shared_ptr<CommandBuffer>> secondaryCommandBuffers;
		std::vector<std::shared_ptr<VertexBuffer>> vertexBuffers;
		std::shared_ptr<IndexBuffer> indexBuffer;

		LineVertex*  buffer = nullptr;
		std::vector<Line> lines;

		uint32_t batchDrawCallIndex = 0;
		uint32_t lineIndexCount = 0;

	};
};