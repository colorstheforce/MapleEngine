//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Game Engine			                    //
//////////////////////////////////////////////////////////////////////////////
#include "CameraControllerComponent.h"
#include "Engine/CameraController.h"
#include "Others/Console.h"

namespace Maple 
{
	CameraControllerComponent::ControllerType CameraControllerComponent::stringToType(const std::string& type)
	{
		if (type == "FPS")
			return ControllerType::FPS;
		if (type == "Editor")
			return ControllerType::EditorCamera;
		return ControllerType::Custom;
	}

	CameraControllerComponent::CameraControllerComponent(ControllerType type)
	{
		setControllerType(type);
	}

	auto CameraControllerComponent::setControllerType(CameraControllerComponent::ControllerType type) -> void
	{
		this->type = type;
		switch (type)
		{
		case ControllerType::FPS:
			LOGW("{0} does not implement", __FUNCTION__);
			break;
		case ControllerType::EditorCamera:
			cameraController = std::make_shared<EditorCameraController>();
			break;
		}
	}

	auto CameraControllerComponent::typeToString(ControllerType type) -> std::string
	{
		switch (type)
		{
		case ControllerType::FPS:
			return "FPS";
		case ControllerType::EditorCamera:
			return "Editor";
		}
		return "Custom";
	}

};

