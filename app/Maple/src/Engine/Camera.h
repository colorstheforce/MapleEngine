
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include <glm/glm.hpp>
#include "Scene/Component/Component.h"
#include "Scene/Component/Transform.h"

#include "Math/Frustum.h"

namespace Maple 
{
	class Camera : public Component
	{
	public:
		Camera() = default;
		Camera(float fov, float near, float far, float aspect);
		Camera(float aspectRatio, float scale);
		Camera(float pitch, float yaw, const glm::vec3& position, float fov, float near, float far, float aspect);
		~Camera() = default;

		inline auto setMouseSensitivity(float value) { mouseSensitivity = value; }
		inline auto setOrthographic(bool ortho) { orthographic = ortho; projectionDirty = true; }
		inline auto isOrthographic() const { return orthographic; }
		inline auto getAspectRatio() const { return aspectRatio; }
		inline auto setAspectRatio(float y) { aspectRatio = y;	projectionDirty = true; };

		auto getProjectionMatrix() -> const glm::mat4&;

		inline auto getFar() const { return far_; }
		inline auto getNear() const { return near_; }
		inline auto setFar(float pFar) { far_ = pFar;	projectionDirty = true; }
		inline auto setNear(float pNear) { near_ = pNear; projectionDirty = true; }
		inline auto getFov() const { return fov; }
		inline auto getScale() const { return scale; }
		inline auto setScale(float scale) { this->scale = scale; projectionDirty = true; }
		inline auto setFov(float fov) { this->fov = fov; projectionDirty = true; }


		auto getFrustum(const glm::mat4& viewMatrix) -> const Frustum&;



		template<typename Archive>
		auto save(Archive& archive) const -> void
		{
			archive(cereal::make_nvp("Scale", scale), cereal::make_nvp("Aspect", aspectRatio), cereal::make_nvp("FOV", fov), cereal::make_nvp("Near", near_), cereal::make_nvp("Far", far_)
				, cereal::make_nvp("Id", entity)
			);
		}

		template<typename Archive>
		auto load(Archive& archive)  -> void
		{
			archive(cereal::make_nvp("Scale", scale), cereal::make_nvp("Aspect", aspectRatio), cereal::make_nvp("FOV", fov), cereal::make_nvp("Near", near_), cereal::make_nvp("Far", far_)
				, cereal::make_nvp("Id", entity)
			);
			projectionDirty = true;
		}

	protected:
		auto updateProjectionMatrix() -> void;

		float aspectRatio = 0.0f;
		float scale = 100.0f;
		float zoom = 1.0f;

		glm::mat4 projMatrix = glm::mat4(1.f);
		bool projectionDirty = false;
	
		float mouseSensitivity = 0.1f;
		bool orthographic = false;

		float fov = 0.0f;
		float near_ = 0.0f;
		float far_ = 0.0f;

		Frustum frustum;
	};

};

