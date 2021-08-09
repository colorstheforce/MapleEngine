
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma  once
#include <string>
#include <imgui.h>
#include <memory>
#include <entt/entt.hpp>
#include "EditorWindow.h"

namespace Maple 
{
	class HierarchyWindow : public EditorWindow
	{
	public:
		HierarchyWindow();
		virtual auto onImGui() -> void override;
		inline auto isDraging() const { return draging; }
	private:
		auto drawName() -> void;
		auto popupWindow() -> void;
		auto drawFilter()-> void;
		auto dragEntity()-> void;
		auto drawNode(const entt::entity& node, entt::registry& registry)-> void;
		auto isParentOfEntity(entt::entity entity, entt::entity child, entt::registry& registry)-> bool;
		auto destroyEntity(entt::entity entity, entt::registry& registry)-> void;
		ImGuiTextFilter hierarchyFilter;
		entt::entity doubleClickedEntity = entt::null;
		entt::entity droppedEntity = entt::null;
		entt::entity recentDroppedEntity = entt::null;
		bool draging = false;
	};
};