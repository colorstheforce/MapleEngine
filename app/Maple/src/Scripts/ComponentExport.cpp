//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "ComponentExport.h"

#include "LuaVirtualMachine.h"
extern "C" {
# include "lua.h"
# include "lauxlib.h"
# include "lualib.h"
}
#include <LuaBridge/LuaBridge.h>
#include <string>
#include <functional>

#include "Scene/Component/Transform.h"
#include "Scene/Component/Component.h"

#include "LuaComponent.h"

#include "Scene/Entity/Entity.h"
#include "Scene/Entity/EntityManager.h"


namespace Maple
{
#define EXPORT_COMPONENTS(Comp) \
		 addFunction("add" #Comp,&Entity::addComponent<Comp>) \
		.addFunction("remove" #Comp, &Entity::removeComponent<Comp>) \
		.addFunction("get" #Comp, &Entity::getComponent<Comp>) \
		.addFunction("getOrAdd" #Comp, &Entity::getOrAddComponent<Comp>) \
		.addFunction("tryGet" #Comp, &Entity::tryGetComponent<Comp>) \
        .addFunction("has" #Comp, &Entity::hasComponent<Comp>) \

	namespace ComponentExport
	{
		auto exportLua(lua_State* L) -> void
		{
			luabridge::getGlobalNamespace(L)
				.beginNamespace("entt")
				.beginClass<entt::registry>("registry")
				.endClass()
				.beginClass<entt::entity>("entity")
				.endClass()
				.endNamespace()


				.beginClass <Entity>("Entity")
				.addConstructor <void (*) (entt::entity, Scene*)>()
				.addConstructor <void (*) ()>()
				.addFunction("valid", &Entity::valid)
				.addFunction("destroy", &Entity::destroy)
				.addFunction("setParent", &Entity::setParent)
				.addFunction("getParent", &Entity::getParent)
				.addFunction("isParent", &Entity::isParent)
				.addFunction("getChildren", &Entity::getChildren)
				.addFunction("setActive", &Entity::setActive)
				.addFunction("isActive", &Entity::isActive)

				.EXPORT_COMPONENTS(NameComponent)
				.EXPORT_COMPONENTS(ActiveComponent)
				.EXPORT_COMPONENTS(Transform)
				.EXPORT_COMPONENTS(LuaComponent)

				.endClass()

				.beginClass<EntityManager>("EntityManager")
				.addFunction("Create", static_cast<Entity(EntityManager::*)()> (&EntityManager::create))
				.addFunction("getRegistry", &EntityManager::getRegistry)
				.endClass()

				.beginClass<NameComponent>("NameComponent")
				.addProperty("name", &NameComponent::name)
				.addFunction("getEntity", &NameComponent::getEntity)
				.endClass()


				.beginClass<ActiveComponent>("ActiveComponent")
				.addProperty("active", &ActiveComponent::active)
				.addFunction("getEntity", &ActiveComponent::getEntity)
				.endClass()

				.beginClass<LuaComponent>("LuaComponent")
				.endClass();

	

		}
	};
};