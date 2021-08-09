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
	class DescriptorSet;
	class UniformBuffer;
	class Texture2D;
	class FrameBuffer;
	class Camera;
	class Material;

	class DeferredOffScreenRenderer : public Renderer
	{
	public:
		DeferredOffScreenRenderer() = default;
		DeferredOffScreenRenderer(uint32_t width, uint32_t height);
		~DeferredOffScreenRenderer();

		auto init(const std::shared_ptr<GBuffer>& buffer) -> void override;
		auto begin() -> void override;
		auto present() -> void override;
		auto end() -> void override;
		auto submit(const RenderCommand& cmd) -> void override;
		auto renderScene() -> void override;
		auto beginScene(Scene* scene) -> void override;
		auto onResize(uint32_t width, uint32_t height) -> void override;

		auto onImGui() -> void override;

	private:

		auto createDefaultMaterial() -> void;
		auto createRendererPass() -> void;
		auto createBuffers() -> void;


		struct UniformBufferObject {
			alignas(16) glm::mat4 projView;
		};

		auto createFrameBuffers() -> void;

		std::shared_ptr<UniformBuffer> uniformBuffer;

		UniformBufferObject systemVsUniformBuffer;

		std::unique_ptr<DescriptorSet> descriptorSet;
		std::unique_ptr<Material> defaultMaterial;

		//##################
		int32_t omniIndex = -1;
	};
};