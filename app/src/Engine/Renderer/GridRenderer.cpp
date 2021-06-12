#include "GridRenderer.h"
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

#include "Engine/Mesh.h"
#include "Engine/GBuffer.h"

#include "Engine/Camera.h"
#include "Scene/Component/Transform.h"

#include "Scene/Scene.h"

#include "Application.h"

namespace Maple 
{

	GridRenderer::GridRenderer(uint32_t width, uint32_t height)
	{
		this->width = width;
		this->height = height;
	}

	GridRenderer::~GridRenderer()
	{
	}

	auto GridRenderer::init(const std::shared_ptr<GBuffer>& buffer) -> void 
	{
		this->gbuffer = buffer;
		memset(&systemUniformBuffer, 0, sizeof(UniformBufferObject));
		memset(&systemUniformBufferFrag, 0, sizeof(UniformBufferObjectFrag));
		auto vertShaderCode = File::read("shaders/spv/GridVert.spv");
		auto fragShaderCode = File::read("shaders/spv/GridFrag.spv");
		shader = Shader::create(vertShaderCode, fragShaderCode);

		quad = Mesh::createPlane(5000, 5000, Maple::UP);

		createRenderPass();
		createPipeline();
		createFrameBuffers();
		createUniformBuffer();
	}

	auto GridRenderer::begin() -> void 
	{
		auto bufferId = renderTexture != nullptr ? 0 : VulkanContext::get()->getSwapChain()->getCurrentBuffer();

		renderPass->beginRenderPass(
			getCommandBuffer(), { 0,0,0,0 },
			frameBuffers[bufferId].get()
			, INLINE, width, height);
	}

	auto GridRenderer::present() -> void 
	{
		pipeline->bind(getCommandBuffer());

		quad->getVertexBuffer()->bind(getCommandBuffer(), pipeline.get());
		quad->getIndexBuffer()->bind(getCommandBuffer());

		bindDescriptorSets(pipeline.get(), getCommandBuffer(), 0, {pipeline->getDescriptorSet()});
		drawIndexed(getCommandBuffer(), DrawType::TRIANGLE, quad->getIndexBuffer()->getCount());

		quad->getVertexBuffer()->unbind();
		quad->getIndexBuffer()->unbind();
	}

	auto GridRenderer::end() -> void 
	{
		renderPass->endRenderpass(getCommandBuffer());
	}

	auto GridRenderer::submit(const RenderCommand& cmd) -> void {}

	auto GridRenderer::renderScene() -> void
	{
		begin();
		uniformBuffer->setData(sizeof(UniformBufferObject), &systemUniformBuffer);
		uniformBufferFrag->setData(sizeof(UniformBufferObjectFrag), &systemUniformBufferFrag);
		present();
		end();
	}

	auto GridRenderer::beginScene(Scene* scene) -> void 
	{
		auto camera = scene->getCamera();

		if (camera.first == nullptr || camera.second == nullptr) {
			return;
		}

		systemUniformBuffer.projView = camera.first->getProjectionMatrix() * glm::inverse(camera.second->getWorldMatrix());
		systemUniformBufferFrag.res = gridRes;
		systemUniformBufferFrag.scale = gridSize;
		systemUniformBufferFrag.cameraPos = glm::vec4(camera.second->getWorldPosition(),1.f);
		systemUniformBufferFrag.maxDistance = maxDistance;
	}

	auto GridRenderer::onResize(uint32_t width, uint32_t height) -> void 
	{
		this->width = width;
		this->height = height;
		frameBuffers.clear();
		createFrameBuffers();
	}

	auto GridRenderer::setRenderTarget(std::shared_ptr<Texture> texture, bool rebuildFramebuffer) -> void
	{
		Renderer::setRenderTarget(texture, rebuildFramebuffer);
		if (rebuildFramebuffer) {
			frameBuffers.clear();
			createFrameBuffers();
		}
	}


	auto GridRenderer::createFrameBuffers() -> void
	{
		frameBuffers.clear();
		auto& buffers = VulkanContext::get()->getSwapChain()->getSwapChainBuffers();

		if (renderTexture)
		{
			FrameBufferInfo info;

			info.attachments = { renderTexture ,gbuffer->getDepthBuffer() };
			info.types = {
				TextureType::COLOR,TextureType::DEPTH
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

				info.attachments = { buffers[i],gbuffer->getDepthBuffer() };
				info.types = {TextureType::COLOR,TextureType::DEPTH };

				info.renderPass = renderPass;
				frameBuffers.emplace_back(FrameBuffer::create(info));
			}
		}
	}

	auto GridRenderer::createRenderPass() -> void
	{
		AttachmentInfo textureTypes[2] =
		{ 
			{TextureType::COLOR, TextureFormat::RGBA8},
			{TextureType::DEPTH, TextureFormat::DEPTH},
		};
		RenderPassInfo info;
		info.attachmentCount = 2;
		info.textureType = textureTypes;
		info.clear = false;
		renderPass = RenderPass::create(info);
	}

	auto GridRenderer::createPipeline() -> void
	{
		PipelineInfo pipelineCreateInfo;
		pipelineCreateInfo.shader = shader;
		pipelineCreateInfo.renderPass = renderPass;
		pipelineCreateInfo.polygonMode = PolygonMode::FILL;
		pipelineCreateInfo.cullMode = CullMode::NONE;
		pipelineCreateInfo.transparencyEnabled = true;
		pipelineCreateInfo.depthBiasEnabled = false;
		pipeline = Pipeline::create(pipelineCreateInfo);
	}

	auto GridRenderer::createUniformBuffer() -> void
	{
		if (uniformBuffer == nullptr)
		{
			uint32_t bufferSize = static_cast<uint32_t>(sizeof(UniformBufferObject));
			uniformBuffer = UniformBuffer::create(bufferSize,nullptr);
		}

		if (uniformBufferFrag == nullptr)
		{
			uint32_t bufferSize = static_cast<uint32_t>(sizeof(UniformBufferObjectFrag));
			uniformBufferFrag = UniformBuffer::create(bufferSize, nullptr);
		}


		BufferInfo bufferInfo;
		bufferInfo.buffer = uniformBuffer;
		bufferInfo.offset = 0;
		bufferInfo.size = sizeof(UniformBufferObject);
		bufferInfo.type =DescriptorType::UNIFORM_BUFFER;
		bufferInfo.shaderType = ShaderType::VERTEX_SHADER;
		bufferInfo.name = "UniformBufferObject";
		bufferInfo.binding = 0;


		BufferInfo bufferInfo2;
		bufferInfo2.name = "UniformBuffer";
		bufferInfo2.buffer = uniformBufferFrag;
		bufferInfo2.offset = 0;
		bufferInfo2.size = sizeof(UniformBufferObjectFrag);
		bufferInfo2.type = DescriptorType::UNIFORM_BUFFER;
		bufferInfo2.binding = 1;
		bufferInfo2.shaderType = ShaderType::FRAGMENT_SHADER;


		pipeline->getDescriptorSet()->update({ bufferInfo,bufferInfo2 });

	}

};
