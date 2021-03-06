

#include "DebugRenderer.h"
#include "LineRenderer.h"
#include "Math/BoundingBox.h"

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

	auto DebugRenderer::drawRect(int32_t x, int32_t y, int32_t width, int32_t height) -> void
	{
		auto right = x + width;
		auto bottom = y + height;

		drawLine({ x,y,0 }, { right,y,0 });
		drawLine({ right,y,0 }, { right,bottom,0 } );
		drawLine({ right,bottom,0 }, { x,bottom,0 });
		drawLine({ x,bottom,0 }, { x,y,0 });
	}

	auto DebugRenderer::drawBox(const BoundingBox& box, const glm::vec4& color) -> void
	{
		glm::vec3 uuu = box.max;
		glm::vec3 lll = box.min;
		glm::vec3 ull(uuu.x, lll.y, lll.z);
		glm::vec3 uul(uuu.x, uuu.y, lll.z);
		glm::vec3 ulu(uuu.x, lll.y, uuu.z);
		glm::vec3 luu(lll.x, uuu.y, uuu.z);
		glm::vec3 llu(lll.x, lll.y, uuu.z);
		glm::vec3 lul(lll.x, uuu.y, lll.z);

	
		drawLine(luu, uuu, color);
		drawLine(lul, uul, color);
		drawLine(llu, ulu, color);
		drawLine(lll, ull, color);
		drawLine(lul, lll, color);
		drawLine(uul, ull, color);
		drawLine(luu, llu, color);
		drawLine(uuu, ulu, color);
		drawLine(lll, llu, color);
		drawLine(ull, ulu, color);
		drawLine(lul, luu, color);
		drawLine(uul, uuu, color);
	}


};