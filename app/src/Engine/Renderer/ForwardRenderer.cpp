
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "ForwardRenderer.h"
#include "Engine/Mesh.h"
#include "Engine/GBuffer.h"
#include "Engine/Vulkan/IndexBuffer.h"
#include "Engine/Vulkan/VertexBuffer.h"
#include "Engine/Vertex.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/VulkanDevice.h"
#include "Engine/Vulkan/VulkanHelper.h"
#include "Engine/Vulkan/VulkanSwapChain.h"
#include "Engine/Vulkan/VulkanPipeline.h"
#include "Engine/Vulkan/VulkanRenderPass.h"
#include "Engine/Vulkan/VulkanShader.h"
#include "Engine/Vulkan/VulkanCommandPool.h"
#include "Engine/Interface/Texture.h"
#include "Engine/Vulkan/VulkanUniformBuffer.h"
#include "Engine/Vulkan/VulkanDescriptorSet.h"
#include "Engine/Vulkan/VulkanCommandBuffer.h"
#include "Engine/Vulkan/VulkanFrameBuffer.h"
#include "Engine/Interface/FrameBuffer.h"

#include "Terrain/QuadCollapseMesh.h"

#include "Engine/Camera.h"
#include "Engine/Light.h"
#include "Engine/Material.h"
#include "Scene/Scene.h"
#include "FileSystem/File.h"
#include "Scene/Component/Light.h"
#include "Application.h"

namespace Maple 
{
	ForwardRenderer::ForwardRenderer(uint32_t width, uint32_t height, bool depthTest)
	{
		this->depthTest = depthTest;
		this->width = width;
		this->height = height;
	}

	ForwardRenderer::~ForwardRenderer()
	{
		
	}


	auto ForwardRenderer::init(const std::shared_ptr<GBuffer> & buffer) -> void
	{
		gbuffer = buffer;
	
		AttachmentInfo infos[] = {
			{TextureType::COLOR,TextureFormat::RGBA8},
			{TextureType::DEPTH,TextureFormat::RGBA8}
		};

		RenderPassInfo info;
		info.attachmentCount = 2;
		info.textureType = infos;
		//info.clear = ;


		renderPass = std::make_shared<VulkanRenderPass>(info);

	

		auto vertShaderCode = File::read("shaders/simpleVert.spv");
		auto fragShaderCode = File::read("shaders/simpleFrag.spv");
		shader = std::make_shared<VulkanShader>(vertShaderCode, fragShaderCode);
		PipelineInfo pipeInfo;
		//pipeInfo.drawType = DrawType::TRIANGLE_STRIP;
		//pipeInfo.polygonMode = PolygonMode::LINE;
		pipeInfo.renderPass = renderPass;
		pipeInfo.shader = shader;
		pipeline = std::make_unique<VulkanPipeline>(pipeInfo);


		createFrameBuffers();

		for (size_t i = 0; i < 1/*VulkanContext::get()->getSwapChain()->getSwapChainImages().size()*/; i++) {
			uniformBuffers.emplace_back(std::make_shared<VulkanUniformBuffer>(sizeof(UniformBufferObject), nullptr));
			lightUniformBuffers.emplace_back(std::make_shared<VulkanUniformBuffer>(sizeof(Light), nullptr));
			materialUniformBuffers.emplace_back(std::make_shared<VulkanUniformBuffer>(sizeof(Material), nullptr));
		}


		

		BufferInfo bufferInfo;
		bufferInfo.buffer = uniformBuffers[0];
		bufferInfo.offset = 0;
		bufferInfo.binding = 0;
		bufferInfo.size = sizeof(UniformBufferObject);
		bufferInfo.type = DescriptorType::UNIFORM_BUFFER;
		pipeline->getDescriptorSet()->update({ bufferInfo});


	/*	commandBuffers.clear();
		for (auto i = 0; i < frameBuffers.size(); i++)
		{
			commandBuffers.emplace_back(std::make_shared<VulkanCommandBuffer>(true));
		}*/
	}

	auto ForwardRenderer::begin() -> void
	{
	/*{
		renderPass->beginRenderPass(commandBuffers[bufferId].get(),
			{ 0,1,1,1 }, frameBuffers[bufferId].get(), SubPassContents::INLINE, width,height);*/
	}

	auto ForwardRenderer::present() -> void
	{
		for (auto& cmd : commandQueue)
		{
			auto& sets = pipeline->getDescriptorSet();

		/*	
			pipeline->bind(commandBuffers[bufferId].get());
			cmd.mesh->getVertexBuffer()->bind(commandBuffers[bufferId].get(), pipeline.get());
			cmd.mesh->getIndexBuffer()->bind(commandBuffers[bufferId].get());
*/

	
			cmd.mesh->getVertexBuffer()->unbind();
			cmd.mesh->getIndexBuffer()->unbind();
		}
	}
	

	auto ForwardRenderer::end() -> void
	{
	/*	renderPass->endRenderpass(commandBuffers[bufferId].get());
		if(renderTexture)
			commandBuffers[bufferId]->execute(true);*/
	}

	auto ForwardRenderer::submit(const RenderCommand& cmd) -> void
	{
		commandQueue.emplace_back(cmd);
	}

	auto ForwardRenderer::renderScene() -> void
	{
		bufferId = 0;
		if (renderTexture == nullptr)
			bufferId = VulkanContext::get()->getSwapChain()->getCurrentBuffer();

		begin();
		present();
		end();

	/*	if (renderTexture == nullptr) {
			app->getRenderDevice()->present(commandBuffers[VulkanContext::get()->getSwapChain()->getCurrentBuffer()].get());
		}*/
	}

	auto ForwardRenderer::beginScene(Scene* scene) -> void
	{
		this->camera = camera;
		auto proj = glm::mat4(1.0); /*camera->getProjection()*/;
		auto view = glm::mat4(1.0); /*camera->getViewMatrix()*/;
		auto size = sizeof(UniformBufferObject);

		systemVsUniformBuffer.model = glm::mat4(1);
		systemVsUniformBuffer.view = view;
		systemVsUniformBuffer.proj = proj;
		uniformBuffers[0]->setData(size, &systemVsUniformBuffer);


		commandQueue.clear();
		//to be optimizated
		

	}

	auto ForwardRenderer::onResize(uint32_t width, uint32_t height) -> void
	{
		this->width = width;
		this->height = height;
		frameBuffers.clear();
		createFrameBuffers();
	}

	auto ForwardRenderer::setRenderTarget(std::shared_ptr <Texture> texture, bool rebuildFramebuffer /*= true*/) -> void
	{
		Renderer::setRenderTarget(texture, rebuildFramebuffer);
		if (rebuildFramebuffer) {
			frameBuffers.clear();
			createFrameBuffers();
		}
	}

	auto ForwardRenderer::updateUniformBuffer(const RenderCommand& cmd) -> void
	{
	
	}

	auto ForwardRenderer::createFrameBuffers() -> void
	{
		frameBuffers.clear();
		auto& buffers = VulkanContext::get()->getSwapChain()->getSwapChainBuffers();
	
		if (renderTexture)
		{
			FrameBufferInfo info;
			
			info.attachments = { renderTexture,gbuffer->getDepthBuffer() };
			info.types = {
				TextureType::COLOR,
				TextureType::DEPTH,
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
				info.height =height;

				if (depthTest)
				{
					info.attachments = { buffers[i],gbuffer->getDepthBuffer() };
					info.types = {
						TextureType::COLOR,
						TextureType::DEPTH,
					};
				}
				else
				{
					info.attachments = { buffers[i] };
					info.types = {
						TextureType::COLOR,
					};
				}

				info.renderPass = renderPass;
				frameBuffers.emplace_back(FrameBuffer::create(info));
			}
		}
	}

};

