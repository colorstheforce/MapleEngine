
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma  once
#include <string>

namespace Maple 
{
	class Scene;

	class EditorWindow 
	{
	public:
		virtual ~EditorWindow() = default;
		virtual auto onImGui() -> void = 0;
		virtual auto onSceneCreated(Scene* scene) -> void {};
	public:
        inline auto& getTitle() const { return title; }
        inline auto setTitle(std::string title) { this->title = title; }

        inline auto isActive() const { return active; }
        inline auto setActive(bool active) { this->active = active; }
	protected:
		std::string title;
		bool active = false;
	};
};