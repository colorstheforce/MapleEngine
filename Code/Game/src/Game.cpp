#include "Main.h"
#include "Application.h"
#include "FileSystem/File.h"

namespace Maple 
{
	class Game : public Application
	{
	public:
		Game(): Application(new DefaultDelegate()){}
		auto init() -> void override 
		{
			Application::init();
			if (File::fileExists("default.scene")) {
				sceneManager->addSceneFromFile("default.scene");
				sceneManager->switchScene("default.scene");
			}
		};
	};
};



Maple::Application* createApplication()
{
	return new Maple::Game();
}