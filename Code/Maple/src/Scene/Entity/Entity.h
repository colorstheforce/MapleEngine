//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Game Engine			                    //

//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Scene/Scene.h"
#include "Scene/SceneGraph.h"
#include "Scene/Component/Component.h"

namespace Maple
{
	class MAPLE_EXPORT Entity final
	{
	public:

		Entity() = default;

		Entity(entt::entity handle, Scene* s)
			: entityHandle(handle)
			, scene(s)
		{
		}

		~Entity()
		{
		}

		template<typename T, typename... Args>
		inline T& addComponent(Args&&... args)
		{
#ifdef MAPLE_DEBUG
			if (hasComponent<T>())
				LOGW("Attempting to add extisting component ");
#endif
			T & t = scene->getRegistry().emplace<T>(entityHandle, std::forward<Args>(args)...);
			t.setEntity(entityHandle);
			return t;
		}

		template<typename T, typename... Args>
		inline T& getOrAddComponent(Args&&... args)
		{
			T & t = scene->getRegistry().get_or_emplace<T>(entityHandle, std::forward<Args>(args)...);
			t.setEntity(entityHandle);
			return t;
		}

		template<typename T, typename... Args>
		inline auto addOrReplaceComponent(Args&&... args)
		{
			T & t = scene->getRegistry().emplace_or_replace<T>(entityHandle, std::forward<Args>(args)...);
			t.setEntity(entityHandle);
		}

		template<typename T>
		inline T& getComponent()
		{
			return scene->getRegistry().get<T>(entityHandle);
		}

		template<typename T>
		inline T* tryGetComponent()
		{
			return scene->getRegistry().try_get<T>(entityHandle);
		}

		template<typename T>
		inline T* tryGetComponentFromParent()
		{
			auto t =  scene->getRegistry().try_get<T>(entityHandle);
			if (t == nullptr) 
			{
				auto parent = getParent();
				while (parent.valid() && t == nullptr) {
					t = parent.tryGetComponent<T>();
					parent = parent.getParent();
				}
			}
			return t;
		}

		template<typename T>
		inline auto hasComponent() const -> bool
		{
			return scene->getRegistry().has<T>(entityHandle);
		}

		template<typename T>
		inline auto removeComponent()
		{
			return scene->getRegistry().remove<T>(entityHandle);
		}

		auto isActive()->bool;
		auto setActive(bool active) -> void;
		auto setParent(const Entity& entity)-> void;
		auto findByPath(const std::string& path) -> Entity;
		auto getChildInChildren(const std::string& name)->Entity;
		auto getParent()->Entity;
		auto getChildren() ->std::vector<Entity>;
		bool isParent(const Entity& potentialParent) const;

		inline operator entt::entity() const { return entityHandle; }
		inline operator uint32_t() const { return (uint32_t)entityHandle; }
		inline operator bool() const { return entityHandle != entt::null && scene; }

		inline auto operator==(const Entity& other) const { return entityHandle == other.entityHandle && scene == other.scene; }
		inline auto operator!=(const Entity& other) const { return !(*this == other); }

		inline auto getHandle() const { return entityHandle; }
		inline auto setHandle(entt::entity en) { entityHandle = en; }
		inline auto getScene() const { return scene; }
		inline auto setScene(Scene* sc) { scene = sc; }

		auto destroy() -> void;
		auto valid() -> bool;

	private:
		entt::entity entityHandle = entt::null;
		Scene* scene = nullptr;
		friend class EntityManager;
	};
};
