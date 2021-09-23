#include "LineRenderer.h"
#include "FileSystem/File.h"
#include "Engine/Vulkan/VulkanContext.h"

#include "Engine/Interface/SwapChain.h"
#include "Engine/Interface/CommandBuffer.h"
#include "Engine/Interface/Shader.h"

#include "Engine/Interface/Texture.h"
#include "Engine/Interface/RenderPass.h"
#include "Engine/Interface/FrameBuffer.h"
#include "Engine/Interface/Pipeline.h"
#include "Engine/Interface/UniformBuffer.h"
#include "Engine/Vulkan/VertexBuffer.h"
#include "Engine/Vulkan/IndexBuffer.h"

#include "Engine/Camera.h"
#include "Scene/Component/Transform.h"

#include "Scene/Scene.h"

#include "Application.h"

namespace Maple 
{


	static const uint32_t MAX_LINES = 10000;
	static const uint32_t MAX_LINE_VERTICES = MAX_LINES * 2;
	static const uint32_t MAX_LINE_INDICES = MAX_LINES * 6;
	static const uint32_t MAX_BATCH_DRAW_CALLS	= 100;
	static const uint32_t RENDERER_LINE_SIZE	= sizeof(LineVertex) * 4;
	static const uint32_t RENDERER_BUFFER_SIZE	= RENDERER_LINE_SIZE * MAX_LINE_VERTICES;


	LineRenderer::LineRenderer(uint32_t width, uint32_t height, bool depthTest)
	{
		this->depthTest = depthTest;
		this->width = width;
		this->height = height;
	}

	LineRenderer::~LineRenderer()
	{
	}

	auto LineRenderer::init(const std::shared_ptr<GBuffer>& buffer) -> void 
	{
		memset(&systemUniformBuffer, 0, sizeof(UniformBufferObject));
		shader = Shader::create("shaders/BatchLine.shader");
		createRenderPass();
		createPipeline();
		createFrameBuffers();
		createUniformBuffer();
	}

	auto LineRenderer::begin() -> void 
	{
		auto bufferId = renderTexture != nullptr ? 0 : VulkanContext::get()->getSwapChain()->getCurrentBuffer();

		renderPass->beginRenderPass(
			getCommandBuffer(), { 0.3,0.3,0.3,1 },
			frameBuffers[bufferId].get()
			, INLINE, width, height);
	}

	auto LineRenderer::present() -> void 
	{
		pipeline->bind(getCommandBuffer());

		vertexBuffers[batchDrawCallIndex]->releasePointer();
		vertexBuffers[batchDrawCallIndex]->unbind();

		indexBuffer->setCount(lineIndexCount);
		vertexBuffers[batchDrawCallIndex]->bind(getCommandBuffer(), pipeline.get());
		indexBuffer->bind(getCommandBuffer());

		bindDescriptorSets(pipeline.get(), getCommandBuffer(), 0, {pipeline->getDescriptorSet()});
		drawIndexed(getCommandBuffer(), DrawType::LINES, lineIndexCount);

		vertexBuffers[batchDrawCallIndex]->unbind();
		indexBuffer->unbind();

		lineIndexCount = 0;

		batchDrawCallIndex++;
	}

	auto LineRenderer::end() -> void 
	{
		renderPass->endRenderpass(getCommandBuffer());
	/*	if (renderTexture)
			commandBuffers[currentBufferID]->execute(true);*/

		batchDrawCallIndex = 0;

		lineIndexCount = 0;
		lines.clear();

	}

	auto LineRenderer::submit(const RenderCommand& cmd) -> void {}

	auto LineRenderer::submit(const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& color) -> void
	{
		lines.push_back({p1,p2,color});
	}

	auto LineRenderer::renderScene() -> void
	{
		begin();
		uniformBuffer->setData(sizeof(UniformBufferObject), &systemUniformBuffer);
		vertexBuffers[batchDrawCallIndex]->bind(getCommandBuffer(), pipeline.get());
		buffer = static_cast<LineVertex*>(vertexBuffers[batchDrawCallIndex]->getPointer()); //map
		submitLines();
		present();
		end();

/*
		if (renderTexture == nullptr) {
			Application::get()->getRenderDevice()->present(commandBuffers[VulkanContext::get()->getSwapChain()->getCurrentBuffer()].get());
		}*/

	}

	auto LineRenderer::beginScene(Scene* scene) -> void 
	{
		auto camera = scene->getCamera();

		if (camera.first == nullptr || camera.second == nullptr) {
			return;
		}

		systemUniformBuffer.projView = camera.first->getProjectionMatrix() * glm::inverse(camera.second->getWorldMatrix());
	}

	auto LineRenderer::onResize(uint32_t width, uint32_t height) -> void 
	{
		this->width = width;
		this->height = height;
		frameBuffers.clear();
		createFrameBuffers();
	}

	auto LineRenderer::setRenderTarget(std::shared_ptr<Texture> texture, bool rebuildFramebuffer) -> void
	{
		Renderer::setRenderTarget(texture, rebuildFramebuffer);
		if (rebuildFramebuffer) {
			frameBuffers.clear();
			createFrameBuffers();
		}
	}

	auto LineRenderer::renderInternal() -> void
	{

	
	}

	auto LineRenderer::clear() -> void
	{
		lines.clear();
	}

	auto LineRenderer::submitLines() -> void
	{
		for (auto & line : lines)
		{
			/*if (lineIndexCount >= MAX_LINE_INDICES)
				flush();*/

			buffer->vertex = line.start;
			buffer->color = line.color;
			buffer++;

			buffer->vertex = line.end;
			buffer->color = line.color;
			buffer++;

			lineIndexCount += 2;
		}
	}

	auto LineRenderer::flush() -> void
	{
		present();
		lines.clear();
		vertexBuffers[batchDrawCallIndex]->bind(nullptr, nullptr);
		buffer =static_cast<LineVertex*>(vertexBuffers[batchDrawCallIndex]->getPointer());
	}

	auto LineRenderer::createFrameBuffers() -> void
	{
		frameBuffers.clear();
		auto& buffers = VulkanContext::get()->getSwapChain()->getSwapChainBuffers();

		if (renderTexture)
		{
			FrameBufferInfo info;

			info.attachments = { renderTexture };
			info.types = {
				TextureType::COLOR,
			};
			info.width = renderTexture->getWidth();
			info.height = renderTexture->getHeight();
			info.renderPass = renderPass;
			info.screenFBO = false;
			frameBuffers.emplace_back(FrameBuffer::create(info));

		}
		else
		{
			for (size_t i = 0; i < buffers.size(); i++) {
				FrameBufferInfo info;
				info.width = width;
				info.height = height;

				info.attachments = { buffers[i] };
				info.types = {TextureType::COLOR};

				info.renderPass = renderPass;
				frameBuffers.emplace_back(FrameBuffer::create(info));
			}
		}
	}

	auto LineRenderer::createRenderPass() -> void
	{
		AttachmentInfo textureTypes[1] =
		{ {TextureType::COLOR, TextureFormat::RGBA8} };
		RenderPassInfo info;
		info.attachmentCount = 1;
		info.textureType = textureTypes;
		info.clear = false;
		renderPass = RenderPass::create(info);
	}

	auto LineRenderer::createPipeline() -> void
	{
		PipelineInfo pipelineCreateInfo;
		pipelineCreateInfo.shader = shader;
		pipelineCreateInfo.renderPass = renderPass;
		pipelineCreateInfo.polygonMode = PolygonMode::FILL;
		pipelineCreateInfo.cullMode = CullMode::NONE;
		pipelineCreateInfo.transparencyEnabled = false;
		pipelineCreateInfo.depthBiasEnabled = true;
		pipelineCreateInfo.drawType = DrawType::LINES;
		pipeline = Pipeline::create(pipelineCreateInfo);
	}

	auto LineRenderer::createUniformBuffer() -> void
	{

		uint32_t bufferSize = static_cast<uint32_t>(sizeof(UniformBufferObject));
		uniformBuffer = UniformBuffer::create(bufferSize, nullptr);

		std::vector<BufferInfo> bufferInfos;

		BufferInfo bufferInfo;
		bufferInfo.buffer = uniformBuffer;
		bufferInfo.offset = 0;
		bufferInfo.size = sizeof(UniformBufferObject);
		bufferInfo.type =DescriptorType::UNIFORM_BUFFER;
		bufferInfo.shaderType = ShaderType::VERTEX_SHADER;
		bufferInfo.name = "UniformBufferObject";
		bufferInfo.binding = 0;

		bufferInfos.push_back(bufferInfo);

		pipeline->getDescriptorSet()->update({ bufferInfo });

		vertexBuffers.resize(100);
		for (auto& vertexBuffer : vertexBuffers)
		{
			vertexBuffer.reset(new VertexBuffer());
			vertexBuffer->resize(RENDERER_BUFFER_SIZE, nullptr);
		}
		std::vector<uint32_t> indices;
		indices.resize(MAX_LINE_INDICES);
		for (int32_t i = 0; i < MAX_LINE_INDICES; i++)
		{
			indices[i] = i;
		}
		indexBuffer.reset(new IndexBuffer(indices.data(), MAX_LINE_INDICES));
	}

};
