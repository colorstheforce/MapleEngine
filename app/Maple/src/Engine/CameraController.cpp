#include "CameraController.h"
#include "Camera.h"
#include "Devices/Input.h"
#include "Math/MathUtils.h"

namespace Maple 
{

	EditorCameraController::EditorCameraController()
	{
		focalPoint = Maple::ZERO;
		velocity = glm::vec3(0.0f);
		mouseSensitivity = 0.00001f;
		zoomDampeningFactor = 0.00001f;
		dampeningFactor = 0.00001f;
		rotateDampeningFactor = 0.0000001f;
	}

	EditorCameraController::~EditorCameraController()
	{
	}

	auto EditorCameraController::handleMouse(Transform& transform, float dt, float xpos, float ypos) -> void 
	{
		if (Input::getInput()->isMouseHeld(KeyCode::MouseKey::ButtonRight))
		{
			mouseSensitivity = 0.01f;
			rotateVelocity = rotateVelocity + (glm::vec2{ xpos,ypos } - previousCurserPos) * mouseSensitivity;
		}


		glm::vec3 euler = glm::degrees(transform.getLocalOrientation());
		float pitch = euler.x - rotateVelocity.y;
		float yaw = euler.y - rotateVelocity.x;

		pitch = std::min(pitch, 89.0f);
		pitch = std::max(pitch, -89.0f);

		previousCurserPos = glm::vec2(xpos, ypos);

		transform.setLocalOrientation(glm::radians(glm::vec3{ pitch, yaw, 0.0f }));

		rotateVelocity = rotateVelocity * std::pow(rotateDampeningFactor, dt);

		updateScroll(transform, Input::getInput()->getScrollOffset(), dt);
	}

	auto EditorCameraController::handleKeyboard(Transform& transform, float dt) -> void 
	{
		float multiplier = 100.0f;

		if (Input::getInput()->isKeyHeld(KeyCode::Id::LeftShift))
		{
			multiplier = 2000.0f;
		}

		cameraSpeed = multiplier * dt;

		if (Input::getInput()->isMouseHeld(KeyCode::MouseKey::ButtonRight))
		{
			if (Input::getInput()->isKeyHeld(KeyCode::Id::W))
			{
				velocity -= transform.getForwardDirection() * cameraSpeed;
			}

			if (Input::getInput()->isKeyHeld(KeyCode::Id::S))
			{
				velocity += transform.getForwardDirection() * cameraSpeed;
			}

			if (Input::getInput()->isKeyHeld(KeyCode::Id::A))
			{
				velocity -= transform.getRightDirection() * cameraSpeed;
			}

			if (Input::getInput()->isKeyHeld(KeyCode::Id::D))
			{
				velocity += transform.getRightDirection() * cameraSpeed;
			}

			if (Input::getInput()->isKeyHeld(KeyCode::Id::Q))
			{
				velocity -= transform.getUpDirection() * cameraSpeed;
			}

			if (Input::getInput()->isKeyHeld(KeyCode::Id::E))
			{
				velocity += transform.getUpDirection() * cameraSpeed;
			}
		}

		if (!MathUtils::equals(velocity, Maple::ZERO))
		{
			glm::vec3 position = transform.getLocalPosition();
			position += velocity * dt;
			transform.setLocalPosition(position);
			velocity = velocity * std::pow(dampeningFactor, dt);
		}
	}

	auto EditorCameraController::updateScroll(Transform& transform, float offset, float dt) -> void 
	{
		if (offset != 0.0f)
		{
			zoomVelocity -= dt * offset * 10.0f;
		}

		if (!MathUtils::equals(zoomVelocity, 0.0f))
		{
			glm::vec3 pos = transform.getLocalPosition();
			pos += transform.getForwardDirection() * zoomVelocity;
			zoomVelocity = zoomVelocity * std::pow(zoomDampeningFactor, dt);

			transform.setLocalPosition(pos);
		}
	}

};
