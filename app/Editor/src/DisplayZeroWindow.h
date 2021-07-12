
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
	class DisplayZeroWindow : public EditorWindow
	{
	public:
		DisplayZeroWindow();
		virtual auto onImGui() -> void ;
		virtual auto resize(uint32_t width, uint32_t height) -> void;
	private:
		auto drawToolBar() -> void;
		uint32_t width;
		uint32_t height;
		std::shared_ptr<Texture2D> previewTexture;
		bool freeAspect = false;
		float aspect = 16 / 9.f;
	};
};