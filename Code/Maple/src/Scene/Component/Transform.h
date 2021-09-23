//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Game Engine			                    //
//////////////////////////////////////////////////////////////////////////////

#pragma once


#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Component.h"

namespace Maple 
{
	class Mesh;

	constexpr glm::vec3 ZERO(0, 0, 0);
	constexpr glm::vec3 LEFT(-1.0f, 0.0f, 0.0f);
	constexpr glm::vec3 RIGHT(1.0f, 0.0f, 0.0f);
	constexpr glm::vec3 UP(0.0f, 1.0f, 0.0f);
	constexpr glm::vec3 DOWN(0.0f, -1.0f, 0.0f);
	constexpr glm::vec3 FORWARD(0.0f, 0.0f, 1.0f);
	constexpr glm::vec3 BACK(0.0f, 0.0f, -1.0f);
	constexpr glm::vec3 ONE(1.0f, 1.0f, 1.0f);


	class MAPLE_EXPORT Transform final : public Component
	{
	public:
		Transform();
		Transform(const glm::mat4 & matrix);
		Transform(const glm::vec3 & position);
		~Transform();

		void setWorldMatrix(const glm::mat4& mat);
		void setLocalTransform(const glm::mat4& localMat);
		void setOffsetTransform(const glm::mat4& localMat);
		void setLocalPosition(const glm::vec3& localPos);
		void setLocalScale(const glm::vec3& localScale);
		void setLocalOrientation(const glm::vec3& rotation);

		inline auto& getWorldMatrix() { if (dirty) updateLocalMatrix();  return worldMatrix; }
		inline auto& getLocalMatrix() { if (dirty) updateLocalMatrix();  return localMatrix; }
		inline auto  getWorldPosition() const { return glm::vec3{ worldMatrix[3][0], worldMatrix[3][1], worldMatrix[3][2] }; }
		inline auto  getWorldOrientation() const { return glm::quat_cast(worldMatrix); };
		inline auto& getLocalPosition() const { return localPosition; }
		inline auto& getLocalScale() const { return localScale; }
		inline auto& getLocalOrientation() const { return localOrientation; }
		inline auto& getOffsetMatrix() const { return offsetMatrix; }

		void resetTransform();
		inline auto hasUpdated() const { return hasUpdate; }
		inline void setHasUpdated(bool set) { hasUpdate = set; }

		void updateLocalMatrix();
		void applyTransform();

		glm::vec3 getUpDirection() const;
		glm::vec3 getRightDirection() const;
		glm::vec3 getForwardDirection() const;
	

		static glm::vec3 getScaleFromMatrix(const glm::mat4& mat);


		template<typename Archive>
		void save(Archive& archive) const
		{
			archive(cereal::make_nvp("Position", localPosition), cereal::make_nvp("Rotation", localOrientation), cereal::make_nvp("Scale", localScale), cereal::make_nvp("Id", entity));
		}

		template<typename Archive>
		void load(Archive& archive)
		{
			archive(cereal::make_nvp("Position", localPosition), cereal::make_nvp("Rotation", localOrientation), cereal::make_nvp("Scale",  localScale), cereal::make_nvp("Id", entity));
			dirty = true;
			initLocalPosition = localPosition;
			initLocalScale = localScale;
			initLocalOrientation = localOrientation;
		}

	protected:
		glm::mat4 localMatrix = glm::mat4(1);
		glm::mat4 worldMatrix = glm::mat4(1);
		glm::mat4 offsetMatrix = glm::mat4(1);



		glm::vec3 localPosition = {};
		glm::vec3 localScale = {};
		glm::vec3 localOrientation = {};

		glm::vec3 initLocalPosition = {};
		glm::vec3 initLocalScale = {};
		glm::vec3 initLocalOrientation = {};


		bool hasUpdate = false;
		bool dirty = false;
	};



	class MAPLE_EXPORT BoneWarpper : public Component
	{
	public:
		Mesh * mesh;
		std::vector<uint32_t> indices;
	};

};



