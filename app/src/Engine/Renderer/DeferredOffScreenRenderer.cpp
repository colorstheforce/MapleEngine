//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                                    //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "DeferredOffScreenRenderer.h"
#include "Engine/Mesh.h"
#include "Engine/GBuffer.h"
#include "Engine/Vertex.h"
#include "Engine/Interface/Texture.h"
#include "Engine/Interface/FrameBuffer.h"
#include "Engine/Interface/Pipeline.h"
#include "Engine/Interface/RenderPass.h"
#include "Engine/Interface/CommandBuffer.h"
#include "Engine/Interface/UniformBuffer.h"
#include "Engine/Interface/SwapChain.h"

#include "Scene/Component/Light.h"
#include "Scene/Component/MeshRenderer.h"


#include "Engine/Camera.h"
#include "Engine/Light.h"
#include "Engine/Material.h"
#include "Scene/Scene.h"
#include "FileSystem/File.h"
#include "Application.h"
#include "Engine/Vulkan/VulkanContext.h"


#include "ImGui/ImGuiHelpers.h"
#include "OmniShadowRenderer.h"

namespace Maple 
{
	DeferredOffScreenRenderer::DeferredOffScreenRenderer(uint32_t width, uint32_t height)
	{
		this->width = width;
		this->height = height;
	}

	DeferredOffScreenRenderer::~DeferredOffScreenRenderer()
	{
		
	}


	auto DeferredOffScreenRenderer::init(const std::shared_ptr<GBuffer> & buffer) -> void
	{
		gbuffer = buffer;
		createDefaultMaterial();
		createRendererPass();
		shader = Shader::create("shaders/DeferredOffScreen.shader");
		PipelineInfo pipeInfo;
		pipeInfo.renderPass = renderPass;
		pipeInfo.shader = shader;
		pipeInfo.cullMode = CullMode::BACK;
		pipeInfo.transparencyEnabled = false;
		pipeInfo.depthBiasEnabled = false;

		pipeline = Pipeline::create(pipeInfo);

		createFrameBuffers();
		createBuffers();

		defaultMaterial->createDescriptorSet(pipeline.get(), 1);
	}

	auto DeferredOffScreenRenderer::begin() -> void
	{
		renderPass->beginRenderPass(getCommandBuffer(),
			{ 0.3,0.3,0.3,1 }, frameBuffers[0].get(), SubPassContents::INLINE, width,height);
		uniformBuffer->setData(sizeof(UniformBufferObject), &systemVsUniformBuffer);
	}

	auto DeferredOffScreenRenderer::present() -> void
	{
		pipeline->bind(getCommandBuffer());
		for (auto& cmd : commandQueue)
		{
			/*if (!cmd.material || !cmd.material->getShader())
				continue;*/

			const auto& sets = pipeline->getDescriptorSet();
			const auto material = cmd.material != nullptr ? cmd.material->getDescriptorSet() : defaultMaterial->getDescriptorSet();
			
			auto& pushConstants = shader->getPushConstants();
			memcpy(pushConstants[0].data.get(), &cmd.transform, sizeof(glm::mat4));
			shader->bindPushConstants(getCommandBuffer(), pipeline.get());

			cmd.mesh->getVertexBuffer()->bind(getCommandBuffer(), pipeline.get());
			cmd.mesh->getIndexBuffer()-> bind(getCommandBuffer());

			bindDescriptorSets(pipeline.get(), getCommandBuffer(), 0, { pipeline->getDescriptorSet(),material});
			drawIndexed(getCommandBuffer(), DrawType::TRIANGLE, cmd.mesh->getIndexBuffer()->getCount(), 0);

			cmd.mesh->getVertexBuffer()->unbind();
			cmd.mesh->getIndexBuffer()->unbind();
		}
	}
	

	auto DeferredOffScreenRenderer::end() -> void
	{
		renderPass->endRenderpass(getCommandBuffer());
	}

	auto DeferredOffScreenRenderer::submit(const RenderCommand& cmd) -> void
	{
		commandQueue.emplace_back(cmd);
	}

	auto DeferredOffScreenRenderer::renderScene() -> void
	{
		begin();
		present();
		end();
	}

	auto DeferredOffScreenRenderer::beginScene(Scene* scene) -> void
	{
		commandQueue.clear();
		auto camera = scene->getCamera();

		if (camera.first != nullptr)
		{
			auto& registry = scene->getRegistry();
			auto group = registry.group<MeshRenderer>(entt::get<Transform>);

			auto view = glm::inverse(camera.second->getWorldMatrix());

			if (omniIndex != -1) {
				auto render = manager->getOmniShadowRenderer();
				view = render->getLightViews()[omniIndex];
			}


			auto projView = camera.first->getProjectionMatrix() * view;

			systemVsUniformBuffer.projView = projView;

			for (auto entity : group)
			{
				const auto& [mesh, trans] = group.get<MeshRenderer, Transform>(entity);
				auto material = mesh.mesh->getMaterial();

				if (material)
				{
					if (material->getDescriptorSet() == nullptr
						|| material->getPipeline() != pipeline.get()
						|| material->getTexturesUpdated())
					{
						material->createDescriptorSet(pipeline.get(), 1);
						material->setTexturesUpdated(false);
					}
				}

				RenderCommand command;
				command.mesh = mesh.mesh.get();
				command.material = material.get();
				command.transform = trans.getWorldMatrix();
				submit(command);
			}
		}
	}

	auto DeferredOffScreenRenderer::onResize(uint32_t width, uint32_t height) -> void
	{
		this->width = width;
		this->height = height;
		frameBuffers.clear();
		createFrameBuffers();
	}


	auto DeferredOffScreenRenderer::onImGui() -> void
	{
		ImGuiHelper::property("Omni Index", omniIndex, -1, 5);
	}

	auto DeferredOffScreenRenderer::createDefaultMaterial() -> void
	{
		defaultMaterial = std::make_unique<Material>();
		MaterialProperties properties;
		properties.albedoColor = glm::vec4(1.f);
		properties.roughnessColor = glm::vec4(0.5f);
		properties.metallicColor = glm::vec4(0.5f);
		properties.usingAlbedoMap = 1.0f;
		properties.usingRoughnessMap = 0.0f;
		properties.usingNormalMap = 0.0f;
		properties.usingMetallicMap = 0.0f;
		

		defaultMaterial->setAlbedoTexture("textures/default.png");

		defaultMaterial->setMaterialProperites(properties);
	

	}

	auto DeferredOffScreenRenderer::createRendererPass() -> void
	{
		AttachmentInfo infos[] = {
			{TextureType::COLOR,gbuffer->getFormat(GBufferTextures::COLOR)},
			{TextureType::COLOR,gbuffer->getFormat(GBufferTextures::POSITION)},
			{TextureType::COLOR,gbuffer->getFormat(GBufferTextures::NORMALS)},
			{TextureType::COLOR,gbuffer->getFormat(GBufferTextures::PBR)},
			{TextureType::DEPTH,TextureFormat::RGBA8}
		};

		RenderPassInfo info;
		info.attachmentCount = 5;
		info.textureType = infos;
		renderPass = RenderPass::create(info);
	}

	auto DeferredOffScreenRenderer::createBuffers() -> void
	{
		if(uniformBuffer == nullptr){
			uniformBuffer = UniformBuffer::create(sizeof(UniformBufferObject),nullptr);
		}

		BufferInfo bufferInfo = {};
		bufferInfo.buffer = uniformBuffer;
		bufferInfo.offset = 0;
		bufferInfo.size = sizeof(UniformBufferObject);
		bufferInfo.type = DescriptorType::UNIFORM_BUFFER;
		bufferInfo.binding = 0;
		bufferInfo.shaderType = ShaderType::VERTEX_SHADER;
		bufferInfo.name = "UniformBufferObject";
		pipeline->getDescriptorSet()->update({ bufferInfo });
	}

	auto DeferredOffScreenRenderer::createFrameBuffers() -> void
	{
		frameBuffers.clear();
		FrameBufferInfo bufferInfo{};
		bufferInfo.width = width;
		bufferInfo.height =height;
		bufferInfo.renderPass = renderPass;

		bufferInfo.types = {
			TextureType::COLOR,
			TextureType::COLOR,
			TextureType::COLOR,
			TextureType::COLOR,
			TextureType::DEPTH
		};

		bufferInfo.attachments = {
			gbuffer->getBuffer(GBufferTextures::COLOR),
			gbuffer->getBuffer(GBufferTextures::POSITION),
			gbuffer->getBuffer(GBufferTextures::NORMALS),
			gbuffer->getBuffer(GBufferTextures::PBR),
			gbuffer->getDepthBuffer()
		};
		frameBuffers.emplace_back(FrameBuffer::create(bufferInfo));
	}

};

