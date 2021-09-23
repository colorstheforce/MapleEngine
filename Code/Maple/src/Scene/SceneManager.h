//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Game Engine			                    //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <string>
#include <queue>
#include <memory>
#include <unordered_map>
#include "Engine/Core.h"

namespace Maple
{
	class Scene;

	class MAPLE_EXPORT SceneManager final
	{
	public:
		SceneManager() = default;
		auto switchScene(const std::string& name)-> void;
		auto apply()-> void;

		inline auto getCurrentScene() const { return currentScene; }
		inline auto getSceneCount() const { return static_cast<uint32_t>(allScenes.size()); }
		inline auto& getScenes() const { return allScenes; }

		inline auto setSwitchScene(bool switching){ switchingScenes = switching; }
		inline auto isSwitchingScene() const  { return switchingScenes; }

		Scene* getSceneByName(const std::string& sceneName);
	
		auto addScene(const std::string& name,Scene * scene)-> void;
		auto addSceneFromFile(const std::string& filePath) -> void;
	protected:
		Scene* currentScene = nullptr;
		std::vector<std::string> sceneFilePaths;
		std::vector<std::string> sceneFilePathsToLoad;

		std::unordered_map<std::string, std::shared_ptr<Scene>> allScenes;

	private:
		bool switchingScenes = false;
		std::string currentName;
	};
}
