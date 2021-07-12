//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Game Engine			                    //
//////////////////////////////////////////////////////////////////////////////

#include "SceneGraph.h"
#include "Component/Component.h"
#include "Component/Transform.h"

namespace Maple 
{
	auto SceneGraph::init(entt::registry& registry) -> void
	{
		registry.on_construct<Hierarchy>().connect<&Hierarchy::onConstruct>();
		registry.on_update<Hierarchy>().connect<&Hierarchy::onUpdate>();
		registry.on_destroy<Hierarchy>().connect<&Hierarchy::onDestroy>();
	}

	auto SceneGraph::disconnectOnConstruct(bool disable, entt::registry& registry)  -> void
	{
		if (disable)
			registry.on_construct<Hierarchy>().disconnect<&Hierarchy::onConstruct>();
		else
			registry.on_construct<Hierarchy>().connect<&Hierarchy::onConstruct>();
	}

	auto SceneGraph::update(entt::registry& registry)  -> void
	{
		auto nonHierarchyView = registry.view<Transform>(entt::exclude<Hierarchy>);

		for (auto entity : nonHierarchyView)
		{
			registry.get<Transform>(entity).setWorldMatrix(glm::mat4{1.f});
		}

		auto view = registry.view<Transform, Hierarchy>();
		for (auto entity : view)
		{
			const auto hierarchy = registry.try_get<Hierarchy>(entity);
			if (hierarchy && hierarchy->getParent() == entt::null)
			{
				//Recursively update children
				updateTransform(entity, registry);
			}
		}

	}

	auto SceneGraph::updateTransform(entt::entity entity, entt::registry& registry)  -> void
	{
		auto hierarchyComponent = registry.try_get<Hierarchy>(entity);
		if (hierarchyComponent)
		{
			auto transform = registry.try_get<Transform>(entity);
			if (transform)
			{
				if (hierarchyComponent->getParent() != entt::null)
				{
					auto parentTransform = registry.try_get<Transform>(hierarchyComponent->getParent());
					if (parentTransform)
					{
						transform->setWorldMatrix(parentTransform->getWorldMatrix());
					}
				}
				else
				{
					transform->setWorldMatrix(glm::mat4{1.f});
				}
			}

			entt::entity child = hierarchyComponent->getFirst();
			while (child != entt::null)
			{
				auto hierarchyComponent = registry.try_get<Hierarchy>(child);
				auto next = hierarchyComponent ? hierarchyComponent->getNext() : entt::null;
				updateTransform(child, registry);
				child = next;
			}
		}
	}
};



