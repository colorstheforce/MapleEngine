//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once

extern "C" {
# include <lua.h>
# include <lauxlib.h>
# include <lualib.h>
}

#include <LuaBridge/LuaBridge.h>
#include <entt/entt.hpp>
#include <string>
#include "MetaFile.h"
#include "Scene/Component/Component.h"

namespace Maple
{
	class Scene;
	class LuaComponent : public Component
	{
	public:
		friend class MetaFile;
		LuaComponent(const std::string& file, Scene* scene);
		LuaComponent();
		~LuaComponent();
		auto onInit() -> void;
		auto onUpdate(float dt) -> void;
		auto reload() -> void;
		auto loadScript() -> void;
		auto onImGui() -> void;
		auto isLoaded() -> bool;
		auto setFilePath(const std::string& fileName) -> void;

		template<typename Archive>
		auto save(Archive& archive) const -> void
		{
			archive(
				cereal::make_nvp("entity", entity),
				cereal::make_nvp("filePath", file)
			);
			metaFile.save(this,file + ".meta");
		}

		template<typename Archive>
		auto load(Archive& archive) -> void
		{
			archive(
				cereal::make_nvp("entity", entity),
				cereal::make_nvp("filePath", file)
			);
			init();
		}


		inline auto& getFileName() const { return file; }
		inline auto setScene(Scene* val) { scene = val; }

	private:

		auto saveNewFile(const std::string & fileName) -> void;
		auto init()-> void;
		std::string file;
		std::string className;

		std::shared_ptr<luabridge::LuaRef> table;
		std::shared_ptr<luabridge::LuaRef> onInitFunc;
		std::shared_ptr<luabridge::LuaRef> onUpdateFunc;
		Scene* scene = nullptr;

		MetaFile metaFile;
	};

};