//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "MonoExporter.h"
#include "Mono.h"
#include "Others/Console.h"
#include "Scene/Component/Transform.h"
#include "Devices/Input.h"

namespace Maple::MonoExporter
{
	struct ExportVector2 { float x, y; };
	struct ExportVector3 { float x, y, z; };

	static auto LogE(MonoString* monoString) -> void
	{
		LOGE("{0}", mono_string_to_utf8(monoString));
	}

	static auto LogE_Float(float v) -> void
	{
		LOGE("{0}", v);
	}


	static auto LogV(MonoString* monoString) -> void
	{
		LOGV("{0}", mono_string_to_utf8(monoString));
	}

	static auto LogV_Float(float v) -> void
	{
		LOGV("{0}", v);
	}

	static auto LogW(MonoString* monoString) -> void
	{
		LOGW("{0}", mono_string_to_utf8(monoString));
	}


	static auto LogW_Float(float v) -> void
	{
		LOGW("{0}", v);
	}

	static auto LogC(MonoString* monoString) -> void
	{
		LOGC("{0}", mono_string_to_utf8(monoString));
	}

	static auto LogC_Float(float v) -> void
	{
		LOGC("{0}", v);
	}


	static ExportVector3 Transform_GetPosition(void* handle)
	{
		Transform* transform = static_cast<Transform*>(handle);
		return ExportVector3{
			transform->getWorldPosition().x,
			transform->getWorldPosition().y,
			transform->getWorldPosition().z
		};
	}

	static void Transform_SetPosition(void* handle, const ExportVector3 & v) {
		static_cast<Transform*>(handle)->setLocalPosition({ v.x, v.y, v.z });
	}

	static auto Input_IsKeyPressed(const KeyCode::Id key) {
		return Input::getInput()->isKeyPressed(key);
	}

	static auto Input_GetMousePosition() {
		return ExportVector2{
			Input::getInput()->getMousePosition().x,
			Input::getInput()->getMousePosition().y
		};
	}

	static auto Input_IsMouseClicked(const KeyCode::MouseKey key) {
		return Input::getInput()->isMouseClicked(key);
	}

	auto exportMono() -> void
	{
		// Debug
		mono_add_internal_call("Maple.Debug::LogE(string)", LogE);
		mono_add_internal_call("Maple.Debug::LogE(single)", LogE_Float);

		mono_add_internal_call("Maple.Debug::LogV(string)", LogV);
		mono_add_internal_call("Maple.Debug::LogV(single)", LogV_Float);

		mono_add_internal_call("Maple.Debug::LogW(string)", LogW);
		mono_add_internal_call("Maple.Debug::LogW(single)", LogW_Float);

		mono_add_internal_call("Maple.Debug::LogC(string)", LogC);
		mono_add_internal_call("Maple.Debug::LogC(single)", LogC_Float);


		// Transform
		mono_add_internal_call("Maple.Transform::_internal_GetPosition()", Transform_GetPosition);
		mono_add_internal_call("Maple.Transform::_internal_SetPosition()", Transform_SetPosition);

		// Input         
		mono_add_internal_call("Maple.Input::IsKeyPressed(Maple.KeyCode)", Input_IsKeyPressed);
		mono_add_internal_call("Maple.Input::IsMouseClicked(Maple.MouseKey)", Input_IsMouseClicked);
		mono_add_internal_call("Maple.Input::GetMousePosition()", Input_GetMousePosition);
	}
};