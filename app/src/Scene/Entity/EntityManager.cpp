//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Game Engine			                    //
//////////////////////////////////////////////////////////////////////////////

#include "EntityManager.h"

namespace Maple
{
	auto EntityManager::create() -> Entity
	{
		return Entity(registry.create(), scene);
	}

	auto EntityManager::create(const std::string& name)  -> Entity
	{
		auto e = registry.create();
		registry.emplace<NameComponent>(e, name);
		return Entity(e, scene);
	}

	auto EntityManager::clear() -> void
	{
		registry.each([&](auto entity) {
			registry.destroy(entity);
		});
		registry.clear();
	}

	auto EntityManager::getEntityByName(const std::string& name)-> Entity
	{
		auto views = registry.view<NameComponent>();
		for (auto & view : views) 
		{
			auto & comp = registry.get<NameComponent>(view);
			if (comp.name == name) {
				return { view,scene };
			}
		}
		return { entt::null,nullptr };
	}

};
