
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "ShadowRenderer.h"
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
	ShadowRenderer::ShadowRenderer(const std::shared_ptr<TextureDepthArray>& texture, uint32_t size , uint32_t numMaps)
		:shadowMapSize(size), numMaps(numMaps)
	{
		shadowTexture = texture ? texture : TextureDepthArray::create(size, size, numMaps);


		memset(&systemUniformBuffer, 0, sizeof(UniformBufferObject));
		init(nullptr);
		app->getRenderManager()->setShadowRenderer(this);
	}

	ShadowRenderer::~ShadowRenderer()
	{

	}

	auto ShadowRenderer::init(const std::shared_ptr<GBuffer> & buffer) -> void
	{
		memset(&systemUniformBuffer, 0, sizeof(UniformBufferObject));

		shader = Shader::create("shaders/Shadow.shader");
		createRenderPass();
		createPipeline();
		createFrameBuffers();
		createUniformBuffer();
	}

	auto ShadowRenderer::begin() -> void
	{
		renderPass->beginRenderPass(getCommandBuffer(),
			{ 0,0,0,1 }, shadowFrameBuffers[cascadeIndex].get(), SubPassContents::INLINE, shadowMapSize, shadowMapSize);
	}

	auto ShadowRenderer::present() -> void
	{
		uniformBuffer->setData(sizeof(UniformBufferObject), &systemUniformBuffer);

		pipeline->bind(getCommandBuffer());

		for (auto& command : cascadeCommandQueue[cascadeIndex])
		{
			Mesh* mesh = command.mesh;

			mesh->getVertexBuffer()->bind(getCommandBuffer(), pipeline.get());
			mesh->getIndexBuffer()->bind(getCommandBuffer());

			auto& pushConstants = shader->getPushConstants();

			glm::ivec4 cascade(cascadeIndex);
			

			memcpy(pushConstants[0].data.get(), &command.transform, sizeof(glm::mat4));
			memcpy(pushConstants[0].data.get() + sizeof(glm::mat4), &cascade,sizeof(glm::ivec4));

			shader->bindPushConstants(getCommandBuffer(), pipeline.get());

			bindDescriptorSets(pipeline.get(), getCommandBuffer(), 0, { pipeline->getDescriptorSet() });
			drawIndexed(getCommandBuffer(), DrawType::TRIANGLE, mesh->getIndexBuffer()->getCount());


			mesh->getVertexBuffer()->unbind();
			mesh->getIndexBuffer()->unbind();
		}

	}

	auto ShadowRenderer::beginScene(Scene* scene) -> void
	{
		hasLight = true;
		auto& registry = scene->getRegistry();
		auto view = registry.group<Light, Transform>();

		Light* light = nullptr;
		for (auto & entity : view)
		{
			auto & clight = view.get<Light>(entity);
			auto & trans = view.get<Transform>(entity);
			if (static_cast<LightType>(clight.lightData.type) == LightType::DirectionalLight) {
				light = &clight;
			}
		}

		auto camera = scene->getCamera();
		if (camera.first == nullptr || camera.second == nullptr || light == nullptr) {
			hasLight = false;
		}

		if (!hasLight)
		{
			return;
		}


		updateCascades(scene, camera.first, camera.second, light);

		for (int32_t i = 0;i<SHADOWMAP_MAX;i++)
		{
			cascadeCommandQueue[i].clear();
		}
	
		auto group = registry.group<MeshRenderer>(entt::get<Transform>);
		for (uint32_t i = 0; i < SHADOWMAP_MAX; ++i)
		{
			if (group.empty())
				continue;

			for (auto entity : group)
			{
				const auto& [render, trans] = group.get<MeshRenderer,Transform>(entity);

				if (render.mesh) 
				{
					auto& worldTransform = trans.getWorldMatrix();
					//TODO calculate if the mesh is in the light view.
					RenderCommand command;
					command.mesh = render.mesh.get();
					command.transform = worldTransform;
					command.material = nullptr;
					submit(command,i);
				}
			}
		}


	}

	auto ShadowRenderer::onImGui() -> void
	{
		ImGuiHelper::property("cascadeSplitLambda", cascadeSplitLambda, 0.01f, 1.f);
		ImGuiHelper::property("bias", initialBias, 0.00001f, 0.5f);
	}

	auto ShadowRenderer::end() -> void
	{
		renderPass->endRenderpass(getCommandBuffer());
	}

	auto ShadowRenderer::submit(const RenderCommand& cmd) -> void
	{
	}

	auto ShadowRenderer::submit(const RenderCommand& cmd,int32_t i) -> void
	{
		cascadeCommandQueue[i].emplace_back(cmd);
	}

	auto ShadowRenderer::renderScene() -> void
	{
		if (hasLight) 
		{
			bufferId = VulkanContext::get()->getSwapChain()->getCurrentBuffer();
			for (int32_t i = 0; i < SHADOWMAP_MAX; i++)
			{
				cascadeIndex = i;
				begin();
				present();
				end();
				///commandBuffers[bufferId]->execute(true);
			}
		}
	}


	auto ShadowRenderer::createFrameBuffers() -> void
	{
		if (shadowMapsInvalidated && numMaps > 0)
		{
			shadowMapsInvalidated = false;

			for (uint32_t i = 0; i < numMaps; ++i)
			{
				FrameBufferInfo bufferInfo{};
				bufferInfo.width = shadowMapSize;
				bufferInfo.height = shadowMapSize;
				bufferInfo.renderPass = renderPass;
				bufferInfo.types = { TextureType::DEPTHARRAY };
				bufferInfo.layer = i;
				bufferInfo.screenFBO = true;
				bufferInfo.attachments = { shadowTexture };

				shadowFrameBuffers[i] = FrameBuffer::create(bufferInfo);
			}
		}
	}

	auto ShadowRenderer::createRenderPass() -> void
	{
		AttachmentInfo textureTypes[1] =
		{ {TextureType::DEPTHARRAY,TextureFormat::DEPTH} };
		RenderPassInfo info;
		info.attachmentCount = 1;
		info.textureType = textureTypes;
		info.clear = true;
		renderPass = RenderPass::create(info);
	}

	auto ShadowRenderer::createPipeline() -> void
	{
		PipelineInfo pipelineCreateInfo;
		pipelineCreateInfo.shader = shader;
		pipelineCreateInfo.renderPass = renderPass;
		pipelineCreateInfo.cullMode = CullMode::NONE;
		pipelineCreateInfo.transparencyEnabled = false;
		pipelineCreateInfo.depthBiasEnabled = true;
		pipeline = Pipeline::create(pipelineCreateInfo);
	}

	auto ShadowRenderer::createUniformBuffer() -> void
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

	auto ShadowRenderer::updateCascades(const Scene* scene, Camera* camera, Transform* transform, const Light* light) -> void
	{
		float cascadeSplits[SHADOWMAP_MAX];

		float nearClip = camera->getNear();
		float farClip = camera->getFar();
		float clipRange = farClip - nearClip;

		float minZ = nearClip;
		float maxZ = nearClip + clipRange;

		float range = maxZ - minZ;
		float ratio = maxZ / minZ;

		// Calculate split depths based on view camera frustum
		// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
		for (uint32_t i = 0; i < SHADOWMAP_MAX; i++) {
			float p = (i + 1) / static_cast<float>(SHADOWMAP_MAX);
			float log = minZ * std::pow(ratio, p);
			float uniform = minZ + range * p;
			float d = cascadeSplitLambda * (log - uniform) + uniform;
			cascadeSplits[i] = (d - nearClip) / clipRange;
		}


		float lastSplitDist = 0.0;
		for (uint32_t i = 0; i < numMaps; i++) {
			float splitDist = cascadeSplits[i];

			auto frum = camera->getFrustum(glm::inverse(transform->getWorldMatrix()));
			glm::vec3* frustumCorners = frum.vertices;
			// Get frustum center

			for (uint32_t i = 0; i < 4; i++) {
				glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
				frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
				frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
			}

			glm::vec3 frustumCenter = glm::vec3(0.0f);
			for (uint32_t i = 0; i < 8; i++) {
				frustumCenter += frustumCorners[i];
			}
			frustumCenter /= 8.0f;

			float radius = 0.0f;
			for (uint32_t i = 0; i < 8; i++) {
				float distance = glm::length(frustumCorners[i] - frustumCenter);
				radius = glm::max(radius, distance);
			}
			radius = std::ceil(radius * 16.0f) / 16.0f;

			glm::vec3 maxExtents = glm::vec3(radius);
			glm::vec3 minExtents = -maxExtents;

			glm::vec3 lightDir = glm::normalize(glm::vec3(light->lightData.direction) * -1.f);
			glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, Maple::UP);
			glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);
			splitDepth[i] = (camera->getNear() + splitDist * clipRange) * -1.f;
			systemUniformBuffer.projView[i] = lightOrthoMatrix * lightViewMatrix;
			lastSplitDist = cascadeSplits[i];
			if (i == 0)
			{
				this->lightViewMatrix = lightViewMatrix;
			}
		}
	}

};

