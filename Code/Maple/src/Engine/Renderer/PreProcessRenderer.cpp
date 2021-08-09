
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "PreProcessRenderer.h"

#include "Engine/Vulkan/VulkanShader.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/VulkanSwapChain.h"
#include "Engine/Vulkan/VulkanRenderPass.h"
#include "Engine/Vulkan/VulkanPipeline.h"
#include "Engine/Vulkan/VulkanDescriptorSet.h"
#include "Engine/Vulkan/VulkanFrameBuffer.h"
#include "Engine/Vulkan/VulkanCommandBuffer.h"
#include "Engine/Vulkan/VulkanUniformBuffer.h"
#include "Engine/VUlkan/VulkanDevice.h"
#include "Engine/Interface/Texture.h"


#include "Engine/Camera.h"
#include "Engine/GBuffer.h"
#include "FileSystem/File.h"
#include "Engine/Mesh.h"
#include "Application.h"
#include "Scene/Scene.h"

#define _USE_MATH_DEFINES
#include <math.h>

namespace Maple
{
	const glm::mat4 captureViews[] =
	{
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};


	PreProcessRenderer::PreProcessRenderer()
	{
	
	}

	PreProcessRenderer::~PreProcessRenderer()
	{
	}

	auto PreProcessRenderer::init(const std::shared_ptr<GBuffer>& buffer) -> void
	{
		manager->setPreProcessRenderer(this);
		gbuffer = buffer;

		shader			 = Shader::create("shaders/CubeMap.shader");
		irradianceShader = Shader::create("shaders/Irradiance.shader");
		prefilterShader  = Shader::create("shaders/Prefilter.shader");


		skyboxCaptureColor = Texture2D::create();
		skyboxCaptureColor->buildTexture(
			TextureFormat::RGBA32, 
			SkyboxSize, 
			SkyboxSize, 
			true, false, false);


		irradianceCaptureColor = Texture2D::create();
		irradianceCaptureColor->buildTexture(
			TextureFormat::RGBA32, 
			Environment::IrradianceMapSize,
			Environment::IrradianceMapSize, 
			true, false, false
		);

		prefilterCaptureColor = Texture2D::create();
		prefilterCaptureColor->buildTexture(
			TextureFormat::RGBA32,
			Environment::PrefilterMapSize,
			Environment::PrefilterMapSize,
			true, false, false
		);

		cube = Mesh::createCube();

		RenderPassInfo info;
		info.attachmentCount = 1;
		AttachmentInfo infos[] = {
			{TextureType::COLOR,TextureFormat::RGBA32}
		};
		info.textureType = infos;
		info.clear = true;

		renderPass = RenderPass::create(info);



		createPipeline();
		updateUniform();
		createFrameBuffer();
	}
	auto PreProcessRenderer::begin() -> void
	{
		renderPass->beginRenderPass(
			commandBuffers[frameBufferId].get(), { 0.3,0.3,0.8,1 },
			frameBuffers[frameBufferId].get(), 
			SubPassContents::INLINE, currentSize, currentSize ,true
		);
	}

	auto PreProcessRenderer::end() -> void
	{
		renderPass->endRenderpass(commandBuffers[frameBufferId].get(),false);
		if (currentTexture) {
			currentTexture->update(commandBuffers[frameBufferId].get(), frameBuffers[frameBufferId].get(), faceId, mip);
		}
		commandBuffers[frameBufferId]->endRecording();
		commandBuffers[frameBufferId]->execute(false);
	}

	auto PreProcessRenderer::renderScene() -> void
	{
		if (envComponent == nullptr) {
			return;
		}
		
		if (envComponent->getEnvironmnet() == nullptr) {
			envComponent->setEnvironmnet(TextureCube::create(SkyboxSize,TextureFormat::RGBA32,5));
		}

		generateSkybox();

		updateIrradianceDescriptor();
		generateIrradianceMap();

		updatePrefilterDescriptor();
		generatePrefilterMap();


		envComponent = nullptr;
	}

	auto PreProcessRenderer::present() -> void
	{
		auto cmd = commandBuffers[frameBufferId].get();

		currentPipeline->bind(cmd);
			//uniform
		auto& constants = currentShader->getPushConstants();

		cube->getVertexBuffer()->bind(cmd, currentPipeline);
		cube->getIndexBuffer()->bind(cmd);
		if (constants.size() > 0)
		{
			memcpy(constants[0].data.get(), &captureViews[faceId], sizeof(glm::mat4));
			currentShader->bindPushConstants(cmd, currentPipeline);
		}

		bindDescriptorSets(currentPipeline, cmd, 0, { currentPipeline->getDescriptorSet() });
		drawIndexed(cmd, DrawType::TRIANGLE, cube->getIndexBuffer()->getCount(), 0);

		cube->getVertexBuffer()->unbind();
		cube->getIndexBuffer()->unbind();
	
	}

	auto PreProcessRenderer::beginScene(Scene* scene) -> void
	{
		auto& registry = scene->getRegistry();
		auto view = registry.view<Environment>();
		if (!view.empty()) {
			auto env = &view.get<Environment>(view.front());
			if (equirectangularMap != env->getEquirectangularMap()) {
				equirectangularMap = env->getEquirectangularMap();
				envComponent = env;
				updateUniform();
			}
		}
	}

	auto PreProcessRenderer::updateIrradianceDescriptor() -> void
	{
		if (irradiancePipeline && envComponent)
		{
			BufferInfo bufferInfo = {};
			bufferInfo.buffer = uniformBufferSkybox;
			bufferInfo.offset = 0;
			bufferInfo.size = sizeof(UniformBufferObjectSkybox);
			bufferInfo.type = DescriptorType::UNIFORM_BUFFER;
			bufferInfo.binding = 0;
			bufferInfo.shaderType = ShaderType::VERTEX_SHADER;

			ImageInfo imageInfo = {};
			imageInfo.textures.emplace_back(envComponent->getEnvironmnet());
			imageInfo.name = "cubemapSampler";
			imageInfo.binding = 1;
			imageInfo.type = TextureType::CUBE;
			irradiancePipeline->getDescriptorSet()->update({ imageInfo }, { bufferInfo });
		}
	}

	auto PreProcessRenderer::updatePrefilterDescriptor() -> void
	{
		if (prefilterPipeline && envComponent)
		{
			BufferInfo bufferInfo = {};
			bufferInfo.buffer = uniformBufferPrefilter;
			bufferInfo.offset = 0;
			bufferInfo.size = sizeof(UniformBufferObjectSkybox);
			bufferInfo.type = DescriptorType::UNIFORM_BUFFER;
			bufferInfo.binding = 0;
			bufferInfo.shaderType = ShaderType::VERTEX_SHADER;
			bufferInfo.name = "UniformBufferObject";


			BufferInfo bufferInfo2 = {};
			bufferInfo2.buffer = uniformBufferPrefilterFrag;
			bufferInfo2.offset = 0;
			bufferInfo2.size = sizeof(UniformBufferObjectRoughness);
			bufferInfo2.type = DescriptorType::UNIFORM_BUFFER;
			bufferInfo2.binding = 1;
			bufferInfo2.name = "UniformBufferObject";
			bufferInfo2.shaderType = ShaderType::FRAGMENT_SHADER;

			ImageInfo imageInfo = {};
			imageInfo.textures.emplace_back(envComponent->getEnvironmnet());
			imageInfo.name = "cubemapSampler";
			imageInfo.binding = 2;
			imageInfo.type = TextureType::CUBE;
			prefilterPipeline->getDescriptorSet()->update({ imageInfo }, { bufferInfo,bufferInfo2 });
		}
	}

	auto PreProcessRenderer::generateSkybox() -> void
	{
		currentSize = SkyboxSize;
		frameBufferId = 0;
		currentPipeline = pipeline.get();
		currentTexture = envComponent->getEnvironmnet().get();
		currentShader = shader.get();
		const auto maxMipLevels = 5;
		uniformBuffer->setData(sizeof(UniformBufferObject), &uniformBufferObj);
		for (mip = 0; mip < maxMipLevels; ++mip)
		{
			currentSize = SkyboxSize * std::pow(0.5, mip);
			for (faceId = 0; faceId < 6; faceId++)
			{
				begin();
				present();
				end();
			}
		}
	}

	auto PreProcessRenderer::generateIrradianceMap() -> void
	{

		currentSize = Environment::IrradianceMapSize;
		frameBufferId = 1;
		currentPipeline = irradiancePipeline.get();
		currentTexture = envComponent->getIrradianceMap().get();
		currentShader = irradianceShader.get();
		mip = 0;
		for (faceId = 0; faceId < 6; faceId++)
		{
			uniformBufferSkyboxObj.view = captureViews[faceId];
			uniformBufferSkyboxObj.proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
			begin();
			uniformBufferSkybox->setData(sizeof(UniformBufferObjectSkybox), &uniformBufferSkyboxObj);
			present();
			end();
		}
	}
	

	auto PreProcessRenderer::generatePrefilterMap() -> void
	{

		currentSize = Environment::PrefilterMapSize;
		frameBufferId = 2;
		currentPipeline = prefilterPipeline.get();
		currentTexture = envComponent->getPrefilteredEnvironment().get();
		currentShader = prefilterShader.get();
		
		const auto maxMipLevels = 5;
		for (mip = 0; mip < maxMipLevels; ++mip)
		{
			currentSize = Environment::PrefilterMapSize * std::pow(0.5, mip);
			roughness.roughness =  (float)mip / (float)(maxMipLevels - 1);
			uniformBufferPrefilterFrag->setData(sizeof(UniformBufferObjectRoughness), &roughness);
			for (faceId = 0; faceId < 6; faceId++)
			{
				uniformBufferSkyboxObj.view = captureViews[faceId];
				uniformBufferSkyboxObj.proj = glm::perspective((float)(M_PI / 2.0f), 1.0f, 0.1f, 10.0f);
				uniformBufferPrefilter->setData(sizeof(UniformBufferObjectSkybox), &uniformBufferSkyboxObj);
				begin();
				present();
				end();
			}
		}
	}

	auto PreProcessRenderer::createPipeline() -> void
	{
		PipelineInfo pipeInfo;
		pipeInfo.renderPass = renderPass;
		pipeInfo.shader = shader;
		pipeInfo.polygonMode = PolygonMode::FILL;
		pipeInfo.cullMode = CullMode::NONE;
		pipeInfo.transparencyEnabled = true;
		pipeInfo.depthBiasEnabled = false;
		pipeline = Pipeline::create(pipeInfo);


		pipeInfo.shader = irradianceShader;
		irradiancePipeline = Pipeline::create(pipeInfo);

		pipeInfo.shader = prefilterShader;
		prefilterPipeline = Pipeline::create(pipeInfo);


		commandBuffers.emplace_back(CommandBuffer::create(true));
		commandBuffers.emplace_back(CommandBuffer::create(true));
		commandBuffers.emplace_back(CommandBuffer::create(true));

	}

	auto PreProcessRenderer::updateUniform() -> void
	{
		if (uniformBufferSkybox == nullptr)
		{
			uniformBufferSkybox = UniformBuffer::create(sizeof(UniformBufferObjectSkybox), nullptr);
		}

		if (uniformBufferPrefilter == nullptr)
		{
			uniformBufferPrefilter = UniformBuffer::create(sizeof(UniformBufferObjectSkybox), nullptr);
		}

		if (uniformBufferPrefilterFrag == nullptr)
		{
			uniformBufferPrefilterFrag = UniformBuffer::create(sizeof(UniformBufferObjectRoughness), nullptr);
		}

		{
			uniformBufferObj.proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
			if (uniformBuffer == nullptr)
			{
				uniformBuffer = UniformBuffer::create(sizeof(UniformBufferObject), nullptr);
			}

			std::vector<BufferInfo> bufferInfos;

			BufferInfo bufferInfo = {};
			bufferInfo.buffer = uniformBuffer;
			bufferInfo.offset = 0;
			bufferInfo.size = sizeof(UniformBufferObject);
			bufferInfo.type = DescriptorType::UNIFORM_BUFFER;
			bufferInfo.binding = 0;
			bufferInfo.shaderType = ShaderType::VERTEX_SHADER;
			bufferInfos.emplace_back(bufferInfo);

			std::vector<ImageInfo> imageInfos;
			if (equirectangularMap)
			{
				ImageInfo imageInfo = {};
				imageInfo.textures.emplace_back(equirectangularMap);
				imageInfo.name = "equirectangularMap";
				imageInfo.binding = 1;
				imageInfo.type = TextureType::COLOR;
				imageInfos.emplace_back(imageInfo);
			}

			if (pipeline)
			{
				pipeline->getDescriptorSet()->update(imageInfos, bufferInfos);
			}
		}
	}

	auto PreProcessRenderer::createFrameBuffer() -> void
	{
		FrameBufferInfo info;
		info.attachments = { skyboxCaptureColor };
		info.types = {
			TextureType::COLOR,
		};
		info.renderPass = renderPass;
		info.width = SkyboxSize;
		info.height = SkyboxSize;
		info.screenFBO = false;
		frameBuffers.emplace_back(FrameBuffer::create(info));


		info.attachments = { irradianceCaptureColor };
		info.width  = Environment::IrradianceMapSize;
		info.height = Environment::IrradianceMapSize;

		frameBuffers.emplace_back(FrameBuffer::create(info));


		info.attachments = { prefilterCaptureColor };
		info.width =  Environment::PrefilterMapSize;
		info.height = Environment::PrefilterMapSize;
		
		frameBuffers.emplace_back(FrameBuffer::create(info));

	}
};

