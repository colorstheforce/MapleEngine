
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "OmniShadowRenderer.h"
#include "Engine/Mesh.h"
#include "Engine/GBuffer.h"

#include "Engine/Vertex.h"

#include "Engine/Interface/RenderPass.h"
#include "Engine/Interface/Texture.h"
#include "Engine/Interface/DescriptorSet.h"
#include "Engine/Interface/UniformBuffer.h"
#include "Engine/Interface/Shader.h"
#include "Engine/Interface/Pipeline.h"
#include "Engine/Interface/SwapChain.h"

#include "Engine/Interface/CommandBuffer.h"
#include "Engine/Interface/FrameBuffer.h"


#include "Engine/Vulkan/VulkanContext.h"

#include "Scene/Component/MeshRenderer.h"


#include "Scene/Component/Transform.h"
#include "Engine/Camera.h"
#include "Engine/Light.h"
#include "Engine/Material.h"
#include "Scene/Scene.h"
#include "FileSystem/File.h"
#include "DeferredOffScreenRenderer.h"

#include "ImGui/ImGuiHelpers.h"

#include "Application.h"

namespace Maple 
{
	OmniShadowRenderer::OmniShadowRenderer(const std::shared_ptr<TextureCube>& texture, uint32_t size)
		:shadowMapSize(size)
	{
		shadowTexture = texture ? texture : TextureCube::create(shadowMapSize);

		memset(&systemUniformBuffer, 0, sizeof(UniformBufferObject));
		init(nullptr);
		manager->setOmniShadowRenderer(this);
	}

	OmniShadowRenderer::~OmniShadowRenderer()
	{

	}

	auto OmniShadowRenderer::init(const std::shared_ptr<GBuffer> & buffer) -> void
	{
		memset(&systemUniformBuffer, 0, sizeof(UniformBufferObject));
		//auto vertShaderCode = File::read("shaders/spv/OmniShadowVert.spv");
		//auto fragShaderCode = File::read("shaders/spv/OmniShadowFrag.spv");
		shader = Shader::create("shaders/OmniShadow.shader");
		createRenderPass();
		createPipeline();
		createFrameBuffers();
		createUniformBuffer();
		float near_ = 1.0f;
		float far_ = 100.f;
		systemUniformBuffer.projView = glm::perspective(glm::radians(90.0f), 1.f, near_, far_);
		systemUniformBuffer.radius = 100;
	}

	auto OmniShadowRenderer::begin() -> void
	{
		renderPass->beginRenderPass(getCommandBuffer(),
			{ 0,0,0,1 }, shadowFrameBuffer.get(), SubPassContents::INLINE, shadowMapSize, shadowMapSize);
	}

	auto OmniShadowRenderer::present() -> void
	{
		uniformBuffer->setData(sizeof(UniformBufferObject), &systemUniformBuffer);

		pipeline->bind(getCommandBuffer());

		for (auto& command : shadowCommandQueue[commandQueueId])
		{
			Mesh* mesh = command.mesh;

			mesh->getVertexBuffer()->bind(getCommandBuffer(), pipeline.get());
			mesh->getIndexBuffer()->bind (getCommandBuffer());

			auto & constants = shader->getPushConstants();

			memcpy(constants[0].data.get(), &command.transform, sizeof(glm::mat4));
			memcpy(constants[0].data.get() + sizeof(glm::mat4), &lightViews[commandQueueId], sizeof(glm::mat4));

			shader->bindPushConstants(getCommandBuffer(), pipeline.get());

			bindDescriptorSets(pipeline.get(), getCommandBuffer(), 0, { pipeline->getDescriptorSet() });
			drawIndexed(getCommandBuffer(), DrawType::TRIANGLE, mesh->getIndexBuffer()->getCount());

			mesh->getVertexBuffer()->unbind();
			mesh->getIndexBuffer()->unbind();
		}

	}

	auto OmniShadowRenderer::beginScene(Scene* scene) -> void
	{
		hasLight = true;
		auto& registry = scene->getRegistry();
		auto view = registry.group<Light, Transform>();

		Light* omniLight = nullptr;
		for (auto & entity : view)
		{
			auto & clight = view.get<Light>(entity);
			auto & trans = view.get<Transform>(entity);

			if (static_cast<LightType>(clight.lightData.type) == LightType::PointLight) {
				omniLight = &clight;
				systemUniformBuffer.lightPos = glm::vec4(trans.getWorldPosition(),1.f);
				systemUniformBuffer.projView = glm::perspective(glm::radians(90.0f), 1.f, 1.f, clight.lightData.radius);
				systemUniformBuffer.radius = clight.lightData.radius;
			}
		}

		auto camera = scene->getCamera();
		if (camera.first == nullptr || camera.second == nullptr || omniLight == nullptr) {
			hasLight = false;
		}

		if (!hasLight)
		{
			return;
		}


		glm::vec3 lightPos = systemUniformBuffer.lightPos;

		for (auto i = 0; i < 6; i++)
		{
			glm::mat4 viewMatrix(1);
			switch (i)
			{
			case 0: // POSITIVE_X
				lightViews[i] = glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
				break;
			case 1:	// NEGATIVE_X
				lightViews[i] = glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
				break;
			case 2:	// POSITIVE_Y
				lightViews[i] = glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
				break;
			case 3:	// NEGATIVE_Y
				lightViews[i] = glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
				break;
			case 4:	// POSITIVE_Z
				lightViews[i] = glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
				break;
			case 5:	// NEGATIVE_Z
				lightViews[i] = glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
				break;
			}
		}
	
		for (auto & q : shadowCommandQueue)
		{
			q.clear();
		}

		auto group = registry.group<MeshRenderer>(entt::get<Transform>);
		for (uint32_t i = 0; i < shadowCommandQueue.size(); ++i)
		{
		
			if (group.empty())
				continue;

			for (auto entity : group)
			{
				const auto& [render, trans] = group.get<MeshRenderer,Transform>(entity);

				if (render.mesh) 
				{
					//TODO calculate if the mesh is in the light view.
					RenderCommand command;
					command.mesh = render.mesh.get();
					command.transform = trans.getWorldMatrix();;
					command.material = nullptr;
					submit(command,i);
				}
			}
		}
	}

	auto OmniShadowRenderer::onImGui() -> void
	{
		//ImGuiHelper::property("cascadeSplitLambda", cascadeSplitLambda, 0.01f, 1.f);
		//ImGuiHelper::property("bias", initialBias, 0.0001f, 0.5f);
	}

	auto OmniShadowRenderer::end() -> void
	{
		renderPass->endRenderpass(getCommandBuffer(),false);
		shadowTexture->update(
			getCommandBuffer(),
			shadowFrameBuffer.get(),
			commandQueueId
		);
	}

	auto OmniShadowRenderer::submit(const RenderCommand& cmd) -> void
	{
	}

	auto OmniShadowRenderer::submit(const RenderCommand& cmd,int32_t i) -> void
	{
		shadowCommandQueue[i].emplace_back(cmd);
	}

	auto OmniShadowRenderer::renderScene() -> void
	{
		if (hasLight) 
		{
			for (int32_t i = 0; i < 6; i++)
			{
				commandQueueId = i;
				begin();
				present();
				end();
			}
		}
	}

	auto OmniShadowRenderer::createFrameBuffers() -> void
	{
		FrameBufferInfo bufferInfo{};
		bufferInfo.width = shadowMapSize;
		bufferInfo.height = shadowMapSize;
		bufferInfo.renderPass = renderPass;
		bufferInfo.types = {TextureType::COLOR,TextureType::DEPTH };
		bufferInfo.layer = 0;
		bufferInfo.screenFBO = true;

		colorAttachment = Texture2D::create();
		colorAttachment->buildTexture(TextureFormat::RGBA8, shadowMapSize, shadowMapSize, false, false, false);

		bufferInfo.attachments = { colorAttachment, TextureDepth::create(shadowMapSize, shadowMapSize) };
		shadowFrameBuffer = FrameBuffer::create(bufferInfo);
	}

	auto OmniShadowRenderer::createRenderPass() -> void
	{
		AttachmentInfo textureTypes[2] =
		{ 
			{TextureType::COLOR,TextureFormat::RGBA8},
			{TextureType::DEPTH,TextureFormat::DEPTH}
		};

		RenderPassInfo info;
		info.attachmentCount = 2;
		info.textureType = textureTypes;
		info.clear = true;
		renderPass = RenderPass::create(info);
	}

	auto OmniShadowRenderer::createPipeline() -> void
	{
		PipelineInfo pipelineCreateInfo;
		pipelineCreateInfo.shader = shader;
		pipelineCreateInfo.renderPass = renderPass;
		pipelineCreateInfo.cullMode = CullMode::BACK;
		pipelineCreateInfo.transparencyEnabled = true;
		pipelineCreateInfo.depthBiasEnabled = false;
		pipeline = Pipeline::create(pipelineCreateInfo);
	}

	auto OmniShadowRenderer::createUniformBuffer() -> void
	{
		if (uniformBuffer == nullptr)
		{
			const uint32_t bufferSize = static_cast<uint32_t>(sizeof(UniformBufferObject));
			uniformBuffer = UniformBuffer::create(bufferSize, nullptr);
		}

		BufferInfo bufferInfo;
		bufferInfo.buffer = uniformBuffer;
		bufferInfo.offset = 0;
		bufferInfo.name = "UniformBufferObject";
		bufferInfo.size = sizeof(UniformBufferObject);
		bufferInfo.type =	DescriptorType::UNIFORM_BUFFER;
		bufferInfo.binding = 0;
		bufferInfo.shaderType = ShaderType::VERTEX_SHADER;
		pipeline->getDescriptorSet()->update({bufferInfo});
	}

};

