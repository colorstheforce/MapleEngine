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
#include "Math/Frustum.h"

namespace Maple
{

	class Texture;
	class Scene;
	class LineRenderer;
	
	class DebugRenderer
	{
	public:
		auto init(uint32_t width, uint32_t height) -> void;
		auto beginScene(Scene* scene) -> void;
		auto begin() -> void;

		auto renderScene() -> void;
		auto setRenderTarget(std::shared_ptr<Texture> texture, bool rebuildFramebuffer) -> void;
		auto onResize(uint32_t width, uint32_t height) -> void;
		auto clear() -> void;

		auto drawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f }) -> void;
		auto drawFrustum(const Frustum & frustum) -> void;

	private:
		LineRenderer* lineRenderer = nullptr;
	};
};