//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Game Engine			                    //
//////////////////////////////////////////////////////////////////////////////

#include <string>
#include "EntityGroup.h"

namespace Maple
{
	class Scene;

	class MAPLE_EXPORT EntityManager final
	{
	public:
		EntityManager(Scene* initScene) : scene(initScene)
		{
		};

		auto create() -> Entity;
		auto create(const std::string& name)->Entity;

		template<typename... Components>
		auto getEntitiesWithTypes()
		{
			return registry.group<Components...>();
		}

		template<typename Component>
		auto getEntitiesWithType() -> EntityView<Component>
		{
			return EntityView<Component>(scene);
		}

		auto getEntityByName(const std::string& name) ->Entity;

		template<typename R, typename T>
		auto addDependency() -> void;
		inline auto& getRegistry(){ return registry; }

		auto clear() -> void;

	private:
		Scene* scene = nullptr;
		entt::registry registry;
	};

	template<typename R, typename T>
	auto EntityManager::addDependency() -> void
	{
		registry.on_construct<R>().connect<&entt::registry::get_or_emplace<T>>();
	}

};