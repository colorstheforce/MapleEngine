//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "HierarchyWindow.h"
#include "Engine/GBuffer.h"
#include "Editor.h"
#include "IconsMaterialDesignIcons.h"
#include "Scene/Scene.h"

#include "Engine/Camera.h"
#include "Scene/Component/Component.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include "Scene/Entity/EntityManager.h"
#include "Scene/Entity/Entity.h"
#include "Scene/Component/Light.h"
#include "Scene/Component/Sprite.h"
#include "Scene/Component/MeshRenderer.h"
#include "Engine/Mesh.h"

#include "ImGui/ImGuiHelpers.h"

#include "imgui_internal.h"

constexpr size_t INPUT_BUFFER = 256;

namespace Maple
{
	HierarchyWindow::HierarchyWindow()
	{
		title = "Hierarchy";
	}

	auto HierarchyWindow::onImGui() -> void 
	{
		const auto flags = ImGuiWindowFlags_NoCollapse;


		ImGui::Begin(title.c_str(), &active, flags);
		{
			if (ImGui::IsMouseClicked(1) && ImGui::IsWindowFocused()) {
				ImGui::OpenPopup("HierarchyWindow::PopupWindow");
			}
			popupWindow();
			drawName();
			drawFilter();
			dragEntity();

		}
		ImGui::End();
	}

	auto HierarchyWindow::drawName()-> void
	{
		auto scene = app->getSceneManager()->getCurrentScene();
		auto& registry = scene->getRegistry();

		const auto& sceneName = scene->getName();

		static char objName[INPUT_BUFFER];
		strcpy(objName, sceneName.c_str());

		ImGui::PushItemWidth(-1);
		if (ImGui::InputText("##Name", objName, IM_ARRAYSIZE(objName), 0))
			scene->setName(objName);
		ImGui::Separator();
	}

	auto HierarchyWindow::popupWindow()-> void
	{
		auto scene = app->getSceneManager()->getCurrentScene();
		auto& registry = scene->getRegistry();
		ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding,10.f);
		if (ImGui::BeginPopupContextWindow("HierarchyWindow::PopupWindow"))
		{
			if (ImGui::Selectable("Add Empty Entity"))
			{
				scene->createEntity("Empty Entity");
			}

			if (ImGui::Selectable("Add Light"))
			{
				auto entity = scene->createEntity("Light");
				entity.addComponent<Light>();
				entity.getOrAddComponent<Transform>();
			}

			const char* shapes[] = { "Sphere", "Cube", "Pyramid", "Capsule", "Cylinder", "Terrain", "Quad" };

			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.f);
			if (ImGui::BeginMenu("Add 3D Object"))
			{
				for (auto name : shapes)
				{
					if (ImGui::MenuItem(name))
					{
						if (strcmp("Cube", name) == 0) {
							auto entity = scene->createEntity(name);
							entity.addComponent<Model>().type = PrimitiveType::Cube;
							auto& meshRender = entity.addComponent<MeshRenderer>(Mesh::createCube());
						}

						if (strcmp("Sphere", name) == 0) {
							auto entity = scene->createEntity(name);
							entity.addComponent<Model>().type = PrimitiveType::Sphere;
							auto& meshRender = entity.addComponent<MeshRenderer>(Mesh::createSphere());
						}
					}
				}
				ImGui::EndMenu();
			}
			ImGui::PopStyleVar();
			if (ImGui::Selectable("Add Camera"))
			{
				auto entity = scene->createEntity("Camera");
				auto& camera = entity.addComponent<Camera>();
				camera.setFov(45.f);
				camera.setFar(100);
				camera.setNear(0.01);
				camera.setAspectRatio(4 / 3.f);
				entity.getOrAddComponent<Transform>();
			}

			if (ImGui::Selectable("Add Sprite"))
			{
				auto entity = scene->createEntity("Sprite");
				entity.addComponent<Sprite>();
				entity.getOrAddComponent<Transform>();
			}

			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
	}

	auto HierarchyWindow::drawFilter()-> void
	{
		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImGui::GetStyleColorVec4(ImGuiCol_TabActive));
		ImGui::TextUnformatted(ICON_MDI_MAGNIFY);
		ImGui::SameLine();
		hierarchyFilter.Draw("##HierarchyFilter", ImGui::GetContentRegionAvail().x - ImGui::GetStyle().IndentSpacing);
		ImGui::PopStyleColor();
		ImGui::Unindent();
	}

	auto HierarchyWindow::dragEntity()-> void
	{
		auto scene = app->getSceneManager()->getCurrentScene();
		auto& registry = scene->getRegistry();

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Drag_Entity"))
			{
				MAPLE_ASSERT(payload->DataSize == sizeof(entt::entity*), "Error ImGUI drag entity");
				auto entity = *reinterpret_cast<entt::entity*>(payload->Data);
				auto hierarchyComponent = registry.try_get<Hierarchy>(entity);
				if (hierarchyComponent)
				{
					Hierarchy::reparent(entity, entt::null, registry, *hierarchyComponent);
				}
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::Indent();

		registry.each([&](auto entity) {
			if (registry.valid(entity))
			{
				auto hierarchyComponent = registry.try_get<Hierarchy>(entity);

				if (!hierarchyComponent || hierarchyComponent->getParent() == entt::null)
					drawNode(entity, registry);
			}
		});

		//Only supports one scene
		ImVec2 minSpace = ImGui::GetWindowContentRegionMin();
		ImVec2 maxSpace = ImGui::GetWindowContentRegionMax();

		float yOffset = std::max(45.0f, ImGui::GetScrollY());
		minSpace.x += ImGui::GetWindowPos().x + 1.0f;
		minSpace.y += ImGui::GetWindowPos().y + 1.0f + yOffset;
		minSpace.x += ImGui::GetWindowPos().x - 1.0f;
		minSpace.y += ImGui::GetWindowPos().y - 1.0f + ImGui::GetScrollY();
		ImRect bb{ minSpace, maxSpace };

		const ImGuiPayload* payload = ImGui::GetDragDropPayload();
		if (payload != NULL && payload->IsDataType("Drag_Entity"))
		{
			bool acceptable = false;

			MAPLE_ASSERT(payload->DataSize == sizeof(entt::entity*), "Error ImGUI drag entity");
			auto entity = *reinterpret_cast<entt::entity*>(payload->Data);
			auto hierarchyComponent = registry.try_get<Hierarchy>(entity);
			if (hierarchyComponent)
			{
				acceptable = hierarchyComponent->getParent() != entt::null;
			}

			if (acceptable && ImGui::BeginDragDropTargetCustom(bb, ImGui::GetID("Panel Hierarchy")))
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Drag_Entity"))
				{
					MAPLE_ASSERT(payload->DataSize == sizeof(entt::entity*), "Error ImGUI drag entity");
					auto entity = *reinterpret_cast<entt::entity*>(payload->Data);
					auto hierarchyComponent = registry.try_get<Hierarchy>(entity);
					if (hierarchyComponent)
					{
						Hierarchy::reparent(entity, entt::null, registry, *hierarchyComponent);
						Entity e(entity, scene);
						e.removeComponent<Hierarchy>();
					}
				}
				ImGui::EndDragDropTarget();
			}
		}
	}

	auto HierarchyWindow::drawNode(const entt::entity& node, entt::registry& registry) -> void
	{
		bool show = true;

		if (!registry.valid(node))
			return;
		Editor* editor = static_cast<Editor*>(app);

		auto scene = app->getSceneManager()->getCurrentScene();


		const auto nameComponent = registry.try_get<NameComponent>(node);
		std::string name = nameComponent ? nameComponent->name : std::to_string(entt::to_integral(node));

		if (hierarchyFilter.IsActive())
		{
			if (!hierarchyFilter.PassFilter(name.c_str()))
			{
				show = false;
			}
		}

		if (show)
		{
			auto hierarchyComponent = registry.try_get<Hierarchy>(node);
			bool noChildren = true;

			if (hierarchyComponent != nullptr && hierarchyComponent->getFirst() != entt::null)
				noChildren = false;

			ImGuiTreeNodeFlags nodeFlags = ((editor->getSelected() == node) ? ImGuiTreeNodeFlags_Selected : 0);

			nodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_FramePadding;

			if (noChildren)
			{
				nodeFlags |= ImGuiTreeNodeFlags_Leaf;
			}

			auto activeComponent = registry.try_get<ActiveComponent>(node);
			bool active = activeComponent ? activeComponent->active : true;

			if (!active)
				ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));

			bool doubleClicked = false;
			if (node == doubleClickedEntity)
			{
				doubleClicked = true;
			}

			if (doubleClicked)
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 1.0f, 2.0f });

			if (recentDroppedEntity == node)
			{
				ImGui::SetNextItemOpen(true);
				recentDroppedEntity = entt::null;
			}

			std::string icon = ICON_MDI_CUBE_OUTLINE;
			auto& iconMap = editor->getComponentIconMap();

			if (registry.has<Light>(node))
			{
				if (iconMap.find(typeid(Light).hash_code()) != iconMap.end())
					icon = iconMap[typeid(Light).hash_code()];
			}
			else if (registry.has<Camera>(node))
			{
				if (iconMap.find(typeid(Camera).hash_code()) != iconMap.end())
					icon = iconMap[typeid(Camera).hash_code()];
			}



			bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)entt::to_integral(node), nodeFlags, (icon + " %s").c_str(), doubleClicked ? "" : (name).c_str());


			if (doubleClicked)
			{
				ImGui::SameLine();
				static char objName[INPUT_BUFFER];
				strcpy(objName, name.c_str());

				ImGui::PushItemWidth(-1);
				if (ImGui::InputText("##Name", objName, IM_ARRAYSIZE(objName), 0))
					registry.get_or_emplace<NameComponent>(node).name = objName;
				ImGui::PopStyleVar();
			}


			if (!active)
				ImGui::PopStyleColor();

			bool deleteEntity = false;
			if (ImGui::BeginPopupContextItem(name.c_str()))
			{
				if (ImGui::Selectable("Copy"))
					editor->setCopiedEntity(node);

				if (ImGui::Selectable("Cut"))
					editor->setCopiedEntity(node, true);

				if (editor->getCopiedEntity() != entt::null && registry.valid(editor->getCopiedEntity()))
				{
					if (ImGui::Selectable("Paste"))
					{
						Entity copiedEntity = { editor->getCopiedEntity(), scene };
						if (!copiedEntity.valid())
						{
							editor->setCopiedEntity(entt::null);
						}
						else
						{
							scene->duplicateEntity(copiedEntity, { node, scene });

							if (editor->isCutCopyEntity())
								deleteEntity = true;
						}
					}
				}
				else
				{
					ImGui::TextDisabled("Paste");
				}

				ImGui::Separator();

				if (ImGui::Selectable("Duplicate"))
				{
					scene->duplicateEntity({ node , scene });
				}
				if (ImGui::Selectable("Delete"))
					deleteEntity = true;
				if (editor->getSelected() == node)
					editor->setSelected(entt::null);
				ImGui::Separator();
				if (ImGui::Selectable("Rename"))
					doubleClickedEntity = node;
				ImGui::Separator();

				if (ImGui::Selectable("Add Child"))
				{
					auto child = scene->getEntityManager()->create();

					child.setParent({ node, scene });
				}
				ImGui::EndPopup();
			}

			draging = false;
			if (!doubleClicked && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
			{
				auto ptr = node;
				ImGui::SetDragDropPayload("Drag_Entity", &ptr, sizeof(entt::entity*));
				ImGui::Text(ICON_MDI_ARROW_UP);
				ImGui::EndDragDropSource();
				draging = true;
			}

			const ImGuiPayload* payload = ImGui::GetDragDropPayload();
			if (payload != NULL && payload->IsDataType("Drag_Entity"))
			{
				draging = true;
				bool acceptable;
				MAPLE_ASSERT(payload->DataSize == sizeof(entt::entity*), "Error ImGUI drag entity");
				auto entity = *reinterpret_cast<entt::entity*>(payload->Data);
				auto hierarchyComponent = registry.try_get<Hierarchy>(entity);
				if (hierarchyComponent != nullptr)
				{
					acceptable = entity != node && (!isParentOfEntity(entity, node, registry)) && (hierarchyComponent->getParent() != node);
				}
				else
					acceptable = entity != node;

				if (ImGui::BeginDragDropTarget())
				{
					// Drop directly on to node and append to the end of it's children list.
					if (ImGui::AcceptDragDropPayload("Drag_Entity"))
					{
						if (acceptable)
						{
							if (hierarchyComponent)
								Hierarchy::reparent(entity, node, registry, *hierarchyComponent);
							else
							{
								registry.emplace<Hierarchy>(entity, node);
							}
							droppedEntity = node;
						}
					}
					ImGui::EndDragDropTarget();
				}
			}

			if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered(ImGuiHoveredFlags_None) && !deleteEntity)
				editor->setSelected(node);
			else if (doubleClickedEntity == node && ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered(ImGuiHoveredFlags_None))
				doubleClickedEntity = entt::null;

			if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered(ImGuiHoveredFlags_None))
			{
				doubleClickedEntity = node;
				/*if (Application::Get().GetEditorState() == EditorState::Preview)
				{
					auto transform = registry.try_get<Maths::Transform>(node);
					if (transform)
						editor.FocusCamera(transform->GetWorldPosition(), 2.0f, 2.0f);
				}*/
			}

			if (deleteEntity)
			{
				destroyEntity(node, registry);
				if (nodeOpen)
					ImGui::TreePop();
				return;
			}

		
			if (nodeOpen == false)
			{
				return;
			}

			const ImColor TreeLineColor = ImColor(128, 128, 128, 128);
			const float SmallOffsetX = 6.0f;
			ImDrawList* drawList = ImGui::GetWindowDrawList();

			ImVec2 verticalLineStart = ImGui::GetCursorScreenPos();
			verticalLineStart.x += SmallOffsetX; //to nicely line up with the arrow symbol
			ImVec2 verticalLineEnd = verticalLineStart;

			if (!noChildren)
			{
				entt::entity child = hierarchyComponent->getFirst();
				while (child != entt::null && registry.valid(child))
				{
					float HorizontalTreeLineSize = 16.0f; //chosen arbitrarily
					auto currentPos = ImGui::GetCursorScreenPos();
					ImGui::Indent(10.0f);

					auto childHerarchyComponent = registry.try_get<Hierarchy>(child);

					if (childHerarchyComponent)
					{
						entt::entity firstChild = childHerarchyComponent->getFirst();
						if (firstChild != entt::null && registry.valid(firstChild))
						{
							HorizontalTreeLineSize *= 0.5f;
						}
					}
					drawNode(child, registry);
					ImGui::Unindent(10.0f);

					const ImRect childRect = ImRect(currentPos, currentPos + ImVec2(0.0f, ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y));

					const float midpoint = (childRect.Min.y + childRect.Max.y) / 2.0f;
					drawList->AddLine(ImVec2(verticalLineStart.x, midpoint), ImVec2(verticalLineStart.x + HorizontalTreeLineSize, midpoint), TreeLineColor);
					verticalLineEnd.y = midpoint;

					if (registry.valid(child))
					{
						auto hierarchyComponent = registry.try_get<Hierarchy>(child);
						child = hierarchyComponent ? hierarchyComponent->getNext() : entt::null;
					}
				}
			}

			drawList->AddLine(verticalLineStart, verticalLineEnd, TreeLineColor);
			ImGui::TreePop();
		}
	}

	auto HierarchyWindow::isParentOfEntity(entt::entity entity, entt::entity child, entt::registry& registry)-> bool
	{
		auto nodeHierarchyComponent = registry.try_get<Hierarchy>(child);
		if (nodeHierarchyComponent)
		{
			auto parent = nodeHierarchyComponent->getParent();
			while (parent != entt::null)
			{
				if (parent == entity)
				{
					return true;
				}
				else
				{
					nodeHierarchyComponent = registry.try_get<Hierarchy>(parent);
					parent = nodeHierarchyComponent ? nodeHierarchyComponent->getParent() : entt::null;
				}
			}
		}
		return false;
	}

	auto HierarchyWindow::destroyEntity(entt::entity entity, entt::registry& registry) -> void
	{
		auto hierarchyComponent = registry.try_get<Hierarchy>(entity);
		if (hierarchyComponent)
		{
			entt::entity child = hierarchyComponent->getFirst();
			while (child != entt::null)
			{
				auto hierarchyComponent = registry.try_get<Hierarchy>(child);
				auto next = hierarchyComponent ? hierarchyComponent->getNext() : entt::null;
				destroyEntity(child, registry);//
				child = next;
			}
		}
		registry.destroy(entity);
	}

};

