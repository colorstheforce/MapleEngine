
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "Renderer2D.h"
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
#include "Scene/Component/Sprite.h"
#include "Engine/Camera.h"
#include "Engine/Light.h"
#include "Engine/Material.h"
#include "Scene/Scene.h"
#include "FileSystem/File.h"
#include "Scene/Component/Light.h"
#include "Application.h"

namespace Maple 
{
	Renderer2D::Renderer2D(uint32_t width, uint32_t height, bool enableDepth)
		:enableDepth(enableDepth)
	{
		this->width = width;
		this->height = height;
		textures.resize(16);
	}

	Renderer2D::~Renderer2D()
	{
		
	}

	auto Renderer2D::init(const std::shared_ptr<GBuffer> & buffer) -> void
	{
		this->gbuffer = buffer;

		AttachmentInfo infos[] = {
			{TextureType::COLOR,TextureFormat::RGBA8},
			{TextureType::DEPTH,TextureFormat::DEPTH}
		};

		RenderPassInfo info;
		info.attachmentCount = enableDepth ? 2 : 1;
		info.textureType = infos;
		info.clear = false;
		renderPass = RenderPass::create(info);
		shader = Shader::create("shaders/Batch2D.shader");

		PipelineInfo pipeInfo;
		pipeInfo.renderPass = renderPass;
		pipeInfo.shader = shader;
		pipeInfo.cullMode = CullMode::NONE;
		pipeInfo.transparencyEnabled = true;
		pipeInfo.depthBiasEnabled = false;
		pipeline = Pipeline::create(pipeInfo);

		createFrameBuffers();

		uniformBuffer = UniformBuffer::create(sizeof(UniformBufferObject),nullptr);

		BufferInfo bufferInfo;
		bufferInfo.buffer = uniformBuffer;
		bufferInfo.offset = 0;
		bufferInfo.binding = 0;
		bufferInfo.size = sizeof(UniformBufferObject);
		bufferInfo.type = DescriptorType::UNIFORM_BUFFER;
		pipeline->getDescriptorSet()->update({ bufferInfo});


		vertexBuffers.resize(config.maxBatchDrawCalls);
		for (int32_t i = 0; i < config.maxBatchDrawCalls; i++)
		{
			vertexBuffers[i] = VertexBuffer::create(BufferUsage::DYNAMIC);
			vertexBuffers[i]->resize(config.bufferSize,nullptr);
		}

		uint32_t* indices = new uint32_t[config.indiciesSize];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < config.indiciesSize; i += 6)
		{
			indices[i] = offset + 0;
			indices[i + 1] = offset + 1;
			indices[i + 2] = offset + 2;

			indices[i + 3] = offset + 2;
			indices[i + 4] = offset + 3;
			indices[i + 5] = offset + 0;

			offset += 4;
		}

		indexBuffer = IndexBuffer::create(indices, config.indiciesSize);
		delete[] indices;


		descriptorSet = DescriptorSet::create({
			pipeline.get(),
			1,
			shader,
			1
		});
	}

	auto Renderer2D::begin() -> void
	{
		auto bufferId = renderTexture != nullptr ? 0 : VulkanContext::get()->getSwapChain()->getCurrentBuffer();
		renderPass->beginRenderPass(
			VulkanContext::get()->getSwapChain()->getCurrentCommandBuffer(),
			{ 1,1,1,1 }, frameBuffers[bufferId].get(), SubPassContents::INLINE, width,height
		);
		textureCount = 0;
		buffer = static_cast<Vertex2D*>(vertexBuffers[batchDrawCallIndex]->getPointer());
	}

	auto Renderer2D::present() -> void
	{
		if (indexCount == 0)
		{
			vertexBuffers[batchDrawCallIndex]->releasePointer();
			return;
		}

		auto cmd = VulkanContext::get()->getSwapChain()->getCurrentCommandBuffer();
		updateDesciptorSet();
		pipeline->bind(cmd);

		indexBuffer->setCount(indexCount);
		indexBuffer->bind(cmd);

		vertexBuffers[batchDrawCallIndex]->releasePointer();
		vertexBuffers[batchDrawCallIndex]->bind(cmd, pipeline.get());
		bindDescriptorSets(pipeline.get(), cmd, 0, { pipeline->getDescriptorSet(),descriptorSet });

		drawIndexed(cmd, DrawType::TRIANGLE, indexCount);

		vertexBuffers[batchDrawCallIndex]->unbind();
		indexBuffer->unbind();

		indexCount = 0;

		batchDrawCallIndex++;

	}
	

	auto Renderer2D::submitQuad() -> void
	{
		uniformBuffer->setData(sizeof(UniformBufferObject), &systemVsUniformBuffer);

		for (auto& command : commands)
		{
			if (indexCount >= config.indiciesSize) {
				flush();
			}
			auto& quad2d = command.quad;
			auto& transform = command.transform;

			const auto& min = quad2d->getPosition();
			const auto& max = quad2d->getPosition() + quad2d->getScale();

			const auto& color = quad2d->getColor();
			const auto& uv = quad2d->getTexCoords();
			const auto texture = quad2d->getTexture();

			int32_t textureSlot = -1;
			if (quad2d->getTexture())
				textureSlot = submitTexture(quad2d->getTexture());

			buffer->vertex = transform * glm::vec4(min.x, min.y, 0.0f, 1);
			buffer->uv = glm::vec3(uv[0], textureSlot);
			buffer->color = color;
			buffer++;

			buffer->vertex = transform * glm::vec4(max.x, min.y, 0.0f, 1);
			buffer->uv = glm::vec3(uv[1], textureSlot);
			buffer->color = color;
			buffer++;

			buffer->vertex = transform * glm::vec4(max.x, max.y, 0.0f, 1.0);
			buffer->uv = glm::vec3(uv[2], textureSlot);
			buffer->color = color;
			buffer++;

			buffer->vertex = transform * glm::vec4(min.x, max.y, 0.0f, 1.0);
			buffer->uv = glm::vec3(uv[3], textureSlot);
			buffer->color = color;
			buffer++;

			indexCount += 6;
		}
	}

	auto Renderer2D::end() -> void
	{
		renderPass->endRenderpass(VulkanContext::get()->getSwapChain()->getCurrentCommandBuffer());
		batchDrawCallIndex = 0;
	}

	auto Renderer2D::submit(const RenderCommand& cmd) -> void
	{
	}

	auto Renderer2D::submit(Quad2D* quad, const glm::mat4 & transform) -> void
	{
		auto & cmd = commands.emplace_back();
		cmd.quad = quad;
		cmd.transform = transform;
	}

	auto Renderer2D::renderScene() -> void
	{
		begin();
		submitQuad();
		present();
		end();
	}

	auto Renderer2D::beginScene(Scene* scene) -> void
	{
		auto camera = scene->getCamera();
		systemVsUniformBuffer.projView = 
			camera.first->getProjectionMatrix() * glm::inverse(camera.second->getWorldMatrix());
	
		commands.clear();
		auto& registry = scene->getRegistry();
		auto group = registry.group<Sprite>(entt::get<Transform>);

		for (auto entity : group)
		{
			const auto& [sprite, trans] = group.get<Sprite, Transform>(entity);
			submit(&sprite.getQuad(), trans.getWorldMatrix());
		}
	}

	auto Renderer2D::onResize(uint32_t width, uint32_t height) -> void
	{
		this->width = width;
		this->height = height;
		frameBuffers.clear();
		createFrameBuffers();
	}

	auto Renderer2D::setRenderTarget(std::shared_ptr <Texture> texture, bool rebuildFramebuffer /*= true*/) -> void
	{
		Renderer::setRenderTarget(texture, rebuildFramebuffer);
		if (rebuildFramebuffer) {
			frameBuffers.clear();
			createFrameBuffers();
		}
	}

	auto Renderer2D::submitTexture(const std::shared_ptr<Texture> & texture) ->int32_t
	{
		float result = 0.0f;
		bool found = false;
		for (uint32_t i = 0; i < textureCount; i++)
		{
			if (textures[i] == texture) //Temp
			{
				return i + 1;
			}
		}
		
		if (textureCount >= config.maxTextures)
		{
			flush();
		}
		textures[textureCount] = texture;
		textureCount++;
		return textureCount;
	}

	auto Renderer2D::updateDesciptorSet() -> void
	{
		if (textureCount == 0)
			return;

		ImageInfo imageInfo{};
		imageInfo.binding = 0;
		imageInfo.name = "textures";
		imageInfo.textures = { textures };
		imageInfo.type = TextureType::COLOR;
		descriptorSet->update({ imageInfo });

	}

	auto Renderer2D::flush() -> void
	{
		present();
		textureCount = 0;
		vertexBuffers[batchDrawCallIndex]->unbind();
		buffer = static_cast<Vertex2D*>(vertexBuffers[batchDrawCallIndex]->getPointer());
	}

	auto Renderer2D::createFrameBuffers() -> void
	{
		frameBuffers.clear();
		auto& buffers = VulkanContext::get()->getSwapChain()->getSwapChainBuffers();
	
		if (renderTexture)
		{
			FrameBufferInfo info;
			
			if (enableDepth) 
			{
				info.attachments = { renderTexture,gbuffer->getDepthBuffer() };
				info.types = {
					TextureType::COLOR,
					TextureType::DEPTH,
				};
			}
			else 
			{
				info.attachments = { renderTexture};
				info.types = {
					TextureType::COLOR,
				};
			}

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

				if (enableDepth)
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

