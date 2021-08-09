
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "SkyboxRenderer.h"
#include "Engine/Vulkan/VulkanShader.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/VulkanSwapChain.h"
#include "Engine/Vulkan/VulkanRenderPass.h"
#include "Engine/Vulkan/VulkanPipeline.h"
#include "Engine/Vulkan/VulkanDescriptorSet.h"
#include "Engine/Vulkan/VulkanFrameBuffer.h"
#include "Engine/Vulkan/VulkanCommandBuffer.h"
#include "Engine/Vulkan/VulkanUniformBuffer.h"
#include "Engine/Vulkan/VulkanDevice.h"
#include "ImGui/ImGuiHelpers.h"
#include "Engine/Camera.h"
#include "Engine/GBuffer.h"
#include "FileSystem/File.h"
#include "Engine/Mesh.h"
#include "Application.h"
#include "Scene/Scene.h"

namespace Maple 
{
	SkyboxRenderer::SkyboxRenderer(uint32_t width, uint32_t height)
	{
		this->width = width;
		this->height = height;
	}
	SkyboxRenderer::~SkyboxRenderer()
	{
	}

	auto SkyboxRenderer::init(const std::shared_ptr<GBuffer>& buffer) -> void
	{
		skybox = Mesh::createCube();
		gbuffer = buffer;
		shader = Shader::create("shaders/Skybox.shader");
	
		AttachmentInfo infos[] = {
			{TextureType::COLOR,TextureFormat::RGBA8},
			{TextureType::DEPTH,TextureFormat::DEPTH}
		};
		RenderPassInfo info;
		info.attachmentCount = 2;
		info.textureType = infos;
		info.clear = false;
		renderPass = RenderPass::create(info);
		createPipeline();
		updateUniform();
		createFrameBuffer();
	}
	auto SkyboxRenderer::begin() -> void 
	{
		auto bufferId = renderTexture != nullptr ? 0 : VulkanContext::get()->getSwapChain()->getCurrentBuffer();

		renderPass->beginRenderPass(
			getCommandBuffer(), { 0.3,0.3,0.3,1 },
			frameBuffers[bufferId].get(), 
			SubPassContents::INLINE, width, height);
	}
	
	auto SkyboxRenderer::end() -> void
	{
		renderPass->endRenderpass(getCommandBuffer());
		/*if (renderTexture) {
			commandBuffers[bufferId]->execute(true);
		}*/
	}
	auto SkyboxRenderer::renderScene() -> void 
	{
		if (!cubeMap) {
			return;
		}

		begin();
		present();
		end();
	}


	auto SkyboxRenderer::present() -> void
	{
		uniformBuffer->setData(sizeof(UniformBufferObject), &uniformBufferObj);
		uniformBufferLodLevel->setData(sizeof(float), &lodLevel);
		//uniform
		pipeline->bind(getCommandBuffer());
		skybox->getVertexBuffer()->bind(getCommandBuffer(), pipeline.get());
		skybox->getIndexBuffer()->bind(getCommandBuffer());

		bindDescriptorSets(pipeline.get(), getCommandBuffer(), 0, { pipeline->getDescriptorSet() });
		drawIndexed(getCommandBuffer(), DrawType::TRIANGLE, skybox->getIndexBuffer()->getCount(), 0);

		skybox->getVertexBuffer()->unbind();
		skybox->getIndexBuffer()->unbind();

	}


	auto SkyboxRenderer::setRenderTarget(std::shared_ptr <Texture> texture, bool rebuildFramebuffer /*= true*/) -> void
	{
		Renderer::setRenderTarget(texture, rebuildFramebuffer);
		if (rebuildFramebuffer) {
			frameBuffers.clear();
			createFrameBuffer();
		}
	}


	auto SkyboxRenderer::beginScene(Scene* scene) -> void 
	{
		auto camera = scene->getCamera();

		if (camera.first != nullptr)  
		{
			uniformBufferObj.proj = camera.first->getProjectionMatrix();
			uniformBufferObj.view = glm::inverse(camera.second->getWorldMatrix());
			uniformBufferObj.view[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

			auto& registry = scene->getRegistry();
			auto view = registry.view<Environment>();
			if (!view.empty()) {
				auto& env = view.get<Environment>(view.front());
				if (cubeMap != env.getEnvironmnet()) {
					setCubeMap(env.getEnvironmnet());
				}
			}
		}
	}

	auto SkyboxRenderer::onResize(uint32_t width, uint32_t height) -> void 
	{
		frameBuffers.clear();
		this->width = width;
		this->height = height;
		//cubeMap = nullptr;
		updateUniform();
		createFrameBuffer();
	}

	auto SkyboxRenderer::setCubeMap(const std::shared_ptr<Texture>& cubeMap) -> void
	{
		this->cubeMap = cubeMap;
		updateUniform();
	}

	auto SkyboxRenderer::createPipeline() -> void
	{
		PipelineInfo pipeInfo;
		pipeInfo.renderPass = renderPass;
		pipeInfo.shader = shader;
		pipeInfo.polygonMode = PolygonMode::FILL;
		pipeInfo.cullMode = CullMode::FRONT;
		pipeInfo.transparencyEnabled = false;
		pipeInfo.depthBiasEnabled = false;

		pipeline = Pipeline::create(pipeInfo);
	}

	auto SkyboxRenderer::updateUniform() -> void
	{
		if (uniformBuffer == nullptr)
		{
			uint32_t bufferSize = static_cast<uint32_t>(sizeof(UniformBufferObject));
			uniformBuffer = UniformBuffer::create(bufferSize, nullptr);
		}

		if (uniformBufferLodLevel == nullptr) 
		{
			uniformBufferLodLevel = UniformBuffer::create(sizeof(float),nullptr);
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

		BufferInfo bufferInfo1 = {};
		bufferInfo1.buffer = uniformBufferLodLevel;
		bufferInfo1.offset = 0;
		bufferInfo1.size = sizeof(float);
		bufferInfo1.type = DescriptorType::UNIFORM_BUFFER;
		bufferInfo1.binding = 2;
		bufferInfo1.shaderType = ShaderType::FRAGMENT_SHADER;
		bufferInfos.emplace_back(bufferInfo1);


		std::vector<ImageInfo> imageInfos;
		if (cubeMap)
		{
			ImageInfo imageInfo = {};
			imageInfo.textures.emplace_back(cubeMap);
			imageInfo.name = "cubeMap";
			imageInfo.binding = 1;
			imageInfo.type = TextureType::CUBE;
			imageInfos.emplace_back(imageInfo);
		}

		if (pipeline)
		{
			pipeline->getDescriptorSet()->update(imageInfos, bufferInfos);
		}

	}

	auto SkyboxRenderer::onImGui() -> void
	{
		ImGuiHelper::property("Skybox Lod", lodLevel, 0.1f, 5.f);
	}

	auto SkyboxRenderer::createFrameBuffer() -> void
	{
		FrameBufferInfo bufferInfo{};
		bufferInfo.width = width;
		bufferInfo.height = height;
		bufferInfo.renderPass = renderPass;
		bufferInfo.types = {
			TextureType::COLOR,
			TextureType::DEPTH
		};;
		

		bufferInfo.attachments.resize(2);
		bufferInfo.attachments[1] = gbuffer->getDepthBuffer();

		if (renderTexture)
		{
			FrameBufferInfo info;
			info.attachments = { renderTexture,gbuffer->getDepthBuffer() };
			info.types = {
				TextureType::COLOR,
				TextureType::DEPTH,
			};
			info.renderPass = renderPass;
			info.width = gbuffer->getDepthBuffer()->getWidth();
			info.height = gbuffer->getDepthBuffer()->getHeight();
			info.screenFBO = false;
			frameBuffers.emplace_back(FrameBuffer::create(info));
		}
		else 
		{
			for (uint32_t i = 0; i < VulkanContext::get()->getSwapChain()->getSwapChainBuffers().size(); i++)
			{
				bufferInfo.screenFBO = true;
				bufferInfo.attachments[0] = VulkanContext::get()->getSwapChain()->getTexture(i);
				frameBuffers.emplace_back(FrameBuffer::create(bufferInfo));
			}
		}
	}

};

