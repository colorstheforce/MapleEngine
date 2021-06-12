
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "DeferredRenderer.h"
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
#include "Engine/Interface/DescriptorSet.h"
#include "Engine/Interface/UniformBuffer.h"

#include "Engine/Interface/CommandBuffer.h"
#include "Engine/Vulkan/VulkanFrameBuffer.h"
#include "Engine/Interface/FrameBuffer.h"

#include "ImGui/ImGuiHelpers.h"

#include "Engine/Camera.h"
#include "Engine/Light.h"
#include "Engine/Material.h"
#include "Scene/Scene.h"
#include "FileSystem/File.h"
#include "DeferredOffScreenRenderer.h"
#include "OmniShadowRenderer.h"
#include "ShadowRenderer.h"
#include "Application.h"

namespace Maple 
{
	DeferredRenderer::DeferredRenderer(uint32_t width, uint32_t height, bool depthTest)
	{
		this->depthTest = depthTest;
		this->width = width;
		this->height = height;
		deferredOffScreenRenderer = new DeferredOffScreenRenderer(width, height);
	}

	DeferredRenderer::~DeferredRenderer()
	{
		delete deferredOffScreenRenderer;
	}


	auto DeferredRenderer::init(const std::shared_ptr<GBuffer> & buffer) -> void
	{
		eventHandler.deferredTypeHandler = [&](auto event) {
			systemVsUniformBuffer.type = event->getDeferredType();
			return true;

		};
		systemVsUniformBuffer.type = 6;
		systemVsUniformBuffer.colorCascade = 0;
		systemVsUniformBuffer.displayCascadeLevel = 0;

		app->getEventDispatcher().addEventHandler(&eventHandler);

		gbuffer = buffer;
		deferredOffScreenRenderer->init(buffer);

		screenQuad = Mesh::createQuad();

		AttachmentInfo infos[] = {
			{TextureType::COLOR,TextureFormat::BGRA8}
		};

		RenderPassInfo info;
		info.attachmentCount = 1;
		info.textureType = infos;
		//info.clear = false;
		 
		renderPass = RenderPass::create(info);

		auto vertShaderCode = File::read("shaders/spv/DeferredLightVert.spv");
		auto fragShaderCode = File::read("shaders/spv/DeferredLightFrag.spv");

		shader = Shader::create(vertShaderCode, fragShaderCode);
		PipelineInfo pipeInfo;
		pipeInfo.renderPass = renderPass;
		pipeInfo.shader = shader;
		pipeInfo.polygonMode = PolygonMode::FILL;
		pipeInfo.cullMode = CullMode::NONE; 
		pipeInfo.transparencyEnabled = false;
		pipeInfo.depthBiasEnabled = false;
		pipeline = Pipeline::create(pipeInfo);

		createFrameBuffers();
		createLightBuffer();

		DescriptorInfo desInfo{};
		desInfo.pipeline = pipeline.get();
		desInfo.layoutIndex = 1;
		desInfo.shader = shader;
		descriptorSet = DescriptorSet::create(desInfo);

		updateScreenDescriptorSet();

	}

	auto DeferredRenderer::begin() -> void
	{
		auto bufferId = renderTexture != nullptr ? 0 : VulkanContext::get()->getSwapChain()->getCurrentBuffer();

		renderPass->beginRenderPass(getCommandBuffer(),
			{ 0,0,0,1 }, frameBuffers[bufferId].get(),
			SubPassContents::INLINE, width,height);
	}

	auto DeferredRenderer::present() -> void
	{
		auto& sets = pipeline->getDescriptorSet();
			
		lightUniformBuffer->setData(sizeof(UniformBufferObject), &systemVsUniformBuffer);
		
		const std::vector<std::shared_ptr<DescriptorSet>> desSets{ sets,descriptorSet };
			
		pipeline->bind(getCommandBuffer());
	
		screenQuad->getVertexBuffer()->bind(getCommandBuffer(), pipeline.get());
		screenQuad->getIndexBuffer() ->bind(getCommandBuffer());


		bindDescriptorSets(pipeline.get(), getCommandBuffer(), 0, desSets);
		drawIndexed(getCommandBuffer(), DrawType::TRIANGLE, screenQuad->getIndexBuffer()->getCount());


		screenQuad->getVertexBuffer()->unbind();
		screenQuad->getIndexBuffer()->unbind();

	}
	

	auto DeferredRenderer::end() -> void
	{
		renderPass->endRenderpass(getCommandBuffer());
		/*if(renderTexture)
			commandBuffers[bufferId]->execute(true);*/
	}

	auto DeferredRenderer::submit(const RenderCommand& cmd) -> void
	{
	}

	auto DeferredRenderer::renderScene() -> void
	{

		deferredOffScreenRenderer->renderScene();

		begin();
		present();
		end();

	}

	auto DeferredRenderer::beginScene(Scene* scene) -> void
	{
		submitLight(scene);
		deferredOffScreenRenderer->beginScene(scene);
	}

	auto DeferredRenderer::onResize(uint32_t width, uint32_t height) -> void
	{
		this->width = width;
		this->height = height;
		frameBuffers.clear();
		createFrameBuffers();
		deferredOffScreenRenderer->onResize(width, height);
		updateScreenDescriptorSet();
	}

	auto DeferredRenderer::setRenderTarget(std::shared_ptr <Texture> texture, bool rebuildFramebuffer /*= true*/) -> void
	{
		Renderer::setRenderTarget(texture, rebuildFramebuffer);
		if (rebuildFramebuffer) {
			frameBuffers.clear();
			createFrameBuffers();
		}
	}

	auto DeferredRenderer::createLightBuffer() -> void
	{
		if (lightUniformBuffer == nullptr)
		{
			lightUniformBuffer = UniformBuffer::create(sizeof(UniformBufferObject),nullptr);
		}

		BufferInfo bufferInfo = {};
		bufferInfo.name = "UniformBufferLight";
		bufferInfo.buffer = lightUniformBuffer;
		bufferInfo.offset = 0;
		bufferInfo.size = sizeof(UniformBufferObject);
		bufferInfo.type = DescriptorType::UNIFORM_BUFFER;
		bufferInfo.binding = 0;
		bufferInfo.shaderType = ShaderType::FRAGMENT_SHADER;
		pipeline->getDescriptorSet()->update({ bufferInfo });
	}

	auto DeferredRenderer::onImGui() -> void
	{
		ImGuiHelper::property("Display Cascade Index", systemVsUniformBuffer.displayCascadeLevel, 0, 3);

		ImGuiHelper::property("Cascade Color", systemVsUniformBuffer.colorCascade, 0, 1);

		if (deferredOffScreenRenderer != nullptr) {
			deferredOffScreenRenderer->onImGui();
		}

	}	

	auto DeferredRenderer::updateScreenDescriptorSet() -> void
	{

		ImageInfo imageInfo = {};
		imageInfo.textures = { gbuffer->getBuffer(GBufferTextures::COLOR) };
		imageInfo.binding = 0;
		imageInfo.type = TextureType::COLOR;
		imageInfo.name = "uColourSampler";

		ImageInfo imageInfo2 = {};
		imageInfo2.textures = { gbuffer->getBuffer(GBufferTextures::POSITION) };
		imageInfo2.binding = 1;
		imageInfo2.type = TextureType::COLOR;
		imageInfo2.name = "uPositionSampler";

		ImageInfo imageInfo3 = {};
		imageInfo3.textures = { gbuffer->getBuffer(GBufferTextures::NORMALS) };
		imageInfo3.binding = 2;
		imageInfo3.type = TextureType::COLOR;
		imageInfo3.name = "uNormalSampler";
	
		ImageInfo imageInfo4 = {};
		imageInfo4.textures = { gbuffer->getDepthBuffer() };
		imageInfo4.binding = 3;
		imageInfo4.type = TextureType::DEPTH;
		imageInfo4.name = "uDepthSampler";

		auto shadowRender = app->getRenderManager()->getShadowRenderer();

		std::vector<ImageInfo> infos{ imageInfo,imageInfo2, imageInfo3, imageInfo4 };

		if (shadowRender) {
			systemVsUniformBuffer.bias = shadowRender->getBias();

			ImageInfo imageInfo5 = {};
			imageInfo5.textures = { shadowRender->getShadowTexture() };
			imageInfo5.binding = 4;
			imageInfo5.type = TextureType::DEPTHARRAY;
			imageInfo5.name = "uShadowMap";
			infos.emplace_back(imageInfo5);
		}


		auto omniShadowRender = app->getRenderManager()->getOmniShadowRenderer();

		if (shadowRender) {
			ImageInfo imageInfo5 = {};
			imageInfo5.textures = { omniShadowRender->getShadowTexture() };
			imageInfo5.binding = 5;
			imageInfo5.type = TextureType::CUBE;
			imageInfo5.name = "uShadowCubeMap";
			infos.emplace_back(imageInfo5);
		}


		descriptorSet->update(infos);
	}

	auto DeferredRenderer::submitLight(Scene* scene) -> void
	{
		auto camera = scene->getCamera();

		if (camera.first != nullptr)
		{
			auto& registry = scene->getRegistry();
			auto lights = registry.view<Light>();
			systemVsUniformBuffer.lightCount = lights.size();
	
			int32_t i = 0;
			for (auto entity : lights)
			{
				auto & [light,trans] = registry.get<Light,Transform>(entity);
				auto forward = trans.getWorldOrientation() * Maple::FORWARD;
				light.lightData.direction = { glm::normalize(forward),1 };
				light.lightData.position =  { trans.getWorldPosition(),1 };
				systemVsUniformBuffer.lights[i++] = light.lightData;
			}
			systemVsUniformBuffer.viewPos = glm::inverse(camera.second->getWorldMatrix());

			if (auto shadowRenderer = app->getRenderManager()->getShadowRenderer())
			{
			
				systemVsUniformBuffer.bias = shadowRenderer->getBias();
				for (auto i = 0;i<SHADOWMAP_MAX;i++)
				{
					systemVsUniformBuffer.lightProjView[i] = shadowRenderer->getShadowCascadeTransform()[i];
					systemVsUniformBuffer.splitDepth[i] = shadowRenderer->getSplitDepth()[i];
				}
			}
		}
	}


	auto DeferredRenderer::createFrameBuffers() -> void
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
				info.types = {
					TextureType::COLOR,
				};

				info.renderPass = renderPass;
				frameBuffers.emplace_back(FrameBuffer::create(info));
			}
		}
	}

};

