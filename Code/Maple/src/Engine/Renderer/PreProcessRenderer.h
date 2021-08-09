//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <cstdint>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "RenderParam.h"
#include "Renderer.h"


namespace Maple
{
	class Mesh;
	class Texture;
	class Texture2D;
	class TextureCube;
	class TextureDepth;
	class UniformBuffer;
	class Environment;

	class PreProcessRenderer : public Renderer
	{
	public:
		static constexpr int32_t SkyboxSize = 512;

		PreProcessRenderer();
		~PreProcessRenderer();

		auto init(const std::shared_ptr<GBuffer>& buffer) -> void override;
		auto begin() -> void override;
		auto present() -> void override;
		auto end() -> void override;
		auto submit(const RenderCommand& cmd) -> void override {};
		auto renderScene() -> void override;
		auto beginScene(Scene* scene) -> void override;

	private:
		auto updateIrradianceDescriptor() -> void;
		auto updatePrefilterDescriptor() -> void;

		auto generateSkybox() -> void;
		auto generateIrradianceMap() -> void;
		auto generatePrefilterMap() -> void;

		struct UniformBufferObject
		{
			alignas(16) glm::mat4 proj;
		};


		struct UniformBufferObjectRoughness
		{
			alignas(16) float roughness;
		}roughness;

		struct UniformBufferObjectSkybox
		{
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};

		auto createPipeline() -> void;
		auto updateUniform() -> void;
		auto createFrameBuffer() -> void;

		std::shared_ptr<Pipeline> irradiancePipeline;
		std::shared_ptr<Pipeline> prefilterPipeline;
		std::shared_ptr<Shader> irradianceShader;
		std::shared_ptr<Shader> prefilterShader;



		Pipeline* currentPipeline = nullptr;


		std::shared_ptr<UniformBuffer> uniformBuffer;
		std::shared_ptr<UniformBuffer> uniformBufferSkybox;
		std::shared_ptr<UniformBuffer> uniformBufferPrefilter;
		std::shared_ptr<UniformBuffer> uniformBufferPrefilterFrag;

		UniformBufferObject uniformBufferObj;
		UniformBufferObjectSkybox uniformBufferSkyboxObj;


		std::shared_ptr<Texture2D> skyboxCaptureColor;
		std::shared_ptr<Texture2D> irradianceCaptureColor;
		std::shared_ptr<Texture2D> prefilterCaptureColor;

		std::shared_ptr<Texture2D> equirectangularMap;
		std::shared_ptr<Mesh> cube;
		Environment* envComponent = nullptr;
		int32_t faceId = 0;

		//0 -- running skybox
		//1 -- irrdiance
		int32_t frameBufferId = 0;
		int32_t currentSize = SkyboxSize;
		TextureCube* currentTexture = nullptr;
		Shader* currentShader = nullptr;
		int32_t mip = 0;


		std::vector<std::shared_ptr<CommandBuffer>> commandBuffers;

	};
};