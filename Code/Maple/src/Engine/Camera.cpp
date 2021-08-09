//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"

namespace Maple
{
	Camera::Camera(float fov, float near, float far, float aspect)
		: aspectRatio(aspect)
		, projectionDirty(true)
		, fov(fov)
		, near_(near)
		, far_(far)
		, orthographic(false)
	{
	}

	Camera::Camera(float aspectRatio, float scale)
		: aspectRatio(aspectRatio)
		, scale(scale)
		, projectionDirty(true)
		, fov(0)
		, near_(1.0)
		, far_(3200000.0f)
		, orthographic(true)
	{

	}

	Camera::Camera(float pitch, float yaw, const glm::vec3& position, float fov, float near, float far, float aspect)
		: aspectRatio(aspect)
		, projectionDirty(true)
		, fov(fov)
		, near_(near)
		, far_(far)
		, orthographic(false)
	{

	}

	auto Camera::getProjectionMatrix() -> const glm::mat4&
	{
		if (projectionDirty)
		{
			updateProjectionMatrix();
			projectionDirty = false;
		}
		return projMatrix;
	}

	auto Camera::getFrustum(const glm::mat4& viewMatrix) -> const Frustum&
	{
		if (projectionDirty) {
			updateProjectionMatrix();
			projectionDirty = false;
		}
		frustum.from(projMatrix * viewMatrix);
		return frustum;
	}

	auto Camera::sendRay(float x, float y, const glm::mat4& viewMatrix, bool invertY /*= false*/) const -> Ray
	{
		Ray ret;
		auto viewProjInverse = glm::inverse(projMatrix * viewMatrix);

		x = 2.0f * x - 1.0f;
		y = 2.0f * y - 1.0f;

		if (invertY)
			y *= -1.0f;
		

		glm::vec4 nearPlane(x, y, 0,1.f);
		glm::vec4 farPlane(x, y,  1, 1.f);

		auto pos = viewProjInverse* nearPlane;
		auto pos2 = viewProjInverse * farPlane;

		ret.origin =  pos / pos.w;
		ret.direction = glm::normalize(glm::vec3(pos2 / pos2.w) - ret.origin);

		return ret;
	}

	auto Camera::updateProjectionMatrix() -> void
	{
		if (orthographic) 
			projMatrix = glm::ortho(aspectRatio * -scale, aspectRatio * scale, -scale, scale, near_, far_);
		else
			projMatrix = glm::perspective(glm::radians(fov), aspectRatio, near_, far_);
	}

};