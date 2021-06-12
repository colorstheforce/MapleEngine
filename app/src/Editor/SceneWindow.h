
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma  once
#include <string>
#include <imgui.h>
#include <memory>

#include "EditorWindow.h"

namespace Maple 
{
	class Texture2D;
	class SceneWindow : public EditorWindow
	{
	public:
		SceneWindow();
		virtual auto onImGui() -> void ;
		virtual auto resize(uint32_t width, uint32_t height) -> void;
	private:
		auto drawToolBar() -> void;
		auto drawGizmos(float width, float height, float xpos, float ypos, Scene* scene) -> void;

		bool showCamera = false;
		uint32_t width;
		uint32_t height;
		std::shared_ptr<Texture2D> previewTexture;
	};
};