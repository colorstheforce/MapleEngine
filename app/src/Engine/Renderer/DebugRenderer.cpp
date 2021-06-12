


#include "DebugRenderer.h"
#include "LineRenderer.h"

namespace Maple 
{
	auto DebugRenderer::init(uint32_t width, uint32_t height) -> void
	{
		lineRenderer = new LineRenderer(width, height, false);
		lineRenderer->init(nullptr);
	}
	auto DebugRenderer::beginScene(Scene* scene) -> void
	{
		lineRenderer->beginScene(scene);
	}

	auto DebugRenderer::begin() -> void
	{
		lineRenderer->begin();
	}

	auto DebugRenderer::renderScene() -> void
	{
		lineRenderer->renderScene();
	}

	auto DebugRenderer::setRenderTarget(std::shared_ptr<Texture> texture, bool rebuildFramebuffer) -> void
	{
		lineRenderer->setRenderTarget(texture, rebuildFramebuffer);
	}


	auto DebugRenderer::onResize(uint32_t width, uint32_t height) -> void
	{
		lineRenderer->onResize(width, height);
	}
	auto DebugRenderer::clear() -> void
	{
		lineRenderer->clear();
	}

	auto DebugRenderer::drawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color ) -> void
	{
		lineRenderer->submit(start, end, color);
	}

	auto DebugRenderer::drawFrustum(const Frustum& frustum) -> void
	{
		auto* vertices = frustum.vertices;

		drawLine(vertices[0], vertices[1]);
		drawLine(vertices[1], vertices[2]);
		drawLine(vertices[2], vertices[3]);
		drawLine(vertices[3], vertices[0]);
		drawLine(vertices[4], vertices[5]);
		drawLine(vertices[5], vertices[6]);
		drawLine(vertices[6], vertices[7]);
		drawLine(vertices[7], vertices[4]);
		drawLine(vertices[0], vertices[4]);
		drawLine(vertices[1], vertices[5]);
		drawLine(vertices[2], vertices[6]);
		drawLine(vertices[3], vertices[7]);
	}

};