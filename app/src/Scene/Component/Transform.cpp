//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Game Engine			                    //
//////////////////////////////////////////////////////////////////////////////

#include "Transform.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Maple
{
	glm::vec3 Transform::getForwardDirection() const
	{
		return getWorldOrientation() * FORWARD;
	}

	glm::vec3 Transform::getRightDirection() const
	{
		return getWorldOrientation() * RIGHT;
	}

	glm::vec3 Transform::getUpDirection() const
	{
		return getWorldOrientation() * UP;
	}


	Transform::Transform()
	{
		localPosition = { 0.0f, 0.0f, 0.0f };
		localOrientation = {};
		localScale = { 1.0f, 1.0f, 1.0f };
		localMatrix = glm::mat4(1.f);
		worldMatrix = glm::mat4(1.f);

		initLocalPosition = localPosition;
		initLocalScale = localScale;
		initLocalOrientation = localOrientation;
	}

	Transform::Transform(const glm::mat4& matrix)
	{
		glm::vec3 skew;
		glm::quat rotation;
		glm::vec4 perspective;
		glm::decompose(matrix, localScale, rotation, localPosition, skew, perspective);
		localOrientation = glm::eulerAngles(rotation);
		localMatrix = matrix;
		worldMatrix = matrix;

		initLocalPosition = localPosition;
		initLocalScale = localScale;
		initLocalOrientation = localOrientation;

	}

	Transform::Transform(const glm::vec3& position)
	{
		localPosition = position;
		localOrientation = {};
		localScale = { 1.0f, 1.0f, 1.0f };
		localMatrix = glm::mat4(1.f);
		worldMatrix = glm::mat4(1.f);
		setLocalPosition(position);

		initLocalPosition = localPosition;
		initLocalScale = localScale;
		initLocalOrientation = localOrientation;
	}

	Transform::~Transform() = default;

	void Transform::setWorldMatrix(const glm::mat4 & mat)
	{
		if (dirty)
			updateLocalMatrix();
		worldMatrix = mat * localMatrix;
	}

	void Transform::setLocalTransform(const glm::mat4& localMat)
	{
		localMatrix = localMat;
		hasUpdate = true;
		applyTransform();//decompose 
	}

	void Transform::setLocalPosition(const glm::vec3& localPos)
	{
		dirty = true;
		localPosition = localPos;
	}

	void Transform::setLocalScale(const glm::vec3& scale)
	{
		dirty = true;
		localScale = scale;
	}

	//void Transform::SetLocalOrientation(const glm::quat& quat)
	void Transform::setLocalOrientation(const glm::vec3& rotation)
	{
		dirty = true;
		localOrientation = rotation;
	}

	void Transform::updateLocalMatrix()
	{
		localMatrix = glm::translate(glm::mat4(1), localPosition);
		localMatrix *= glm::toMat4(glm::quat(localOrientation));
		localMatrix = glm::scale(localMatrix,localScale);
		dirty = false;
		hasUpdate = true;
	}

	void Transform::applyTransform()
	{
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::quat rotation;
		glm::decompose(localMatrix, localScale, rotation, localPosition, skew, perspective);
		localOrientation = glm::eulerAngles(rotation);
	}

	glm::vec3 Transform::getScaleFromMatrix(const glm::mat4& mat)
	{
		glm::vec3 skew;
		glm::vec3 localScale;
		glm::quat localOrientation;
		glm::vec3 localPosition;
		glm::vec4 perspective;
		glm::decompose(mat, localScale, localOrientation, localPosition, skew, perspective);
		return localScale;
	}

	void Transform::setOffsetTransform(const glm::mat4& localMat)
	{
		offsetMatrix = localMat;
	}

	void Transform::resetTransform()
	{
		dirty = true;
		localPosition = initLocalPosition;
		localScale = initLocalScale;
		localOrientation = initLocalOrientation;
	}

};

