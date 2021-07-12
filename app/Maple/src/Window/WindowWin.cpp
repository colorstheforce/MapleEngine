//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "WindowWin.h"
#include "Engine/Renderer/VkRenderDevice.h"
#include "Event/Event.h"
#include "Event/WindowEvent.h"
#include "Application.h"
#include <memory>


#define GLFW_INCLUDE_VULKAN
#include "GL/glew.h"
#include "GLFW/glfw3.h"


#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>   // for glfwGetWin32Window
#endif // _WIN32



#define CAST_TO_WIN(x) static_cast<WindowWin*>(glfwGetWindowUserPointer(x))

namespace Maple 
{
	WindowWin::WindowWin(const WindowInitData& initData) : data(initData)
	{
	
	}

	auto WindowWin::onUpdate() -> void
	{
		glfwPollEvents();
	}

	auto WindowWin::setVSync(bool vsync) -> void 
	{
		glfwSwapInterval(vsync ? 1 : 0);
	}

	auto WindowWin::setTitle(const std::string& title) ->void
	{
		glfwSetWindowTitle(nativeInterface, title.c_str());
	}

	auto WindowWin::init() -> void
	{
		if (!glfwInit())
			return;
	
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		nativeInterface = glfwCreateWindow(data.width,data.height,data.title.c_str(),nullptr,nullptr);

		if (!nativeInterface)
			return;

		glfwMakeContextCurrent(nativeInterface);
		glfwSetInputMode(nativeInterface, GLFW_STICKY_KEYS, 1);
		glfwSetWindowUserPointer(nativeInterface, this);
		registerNativeEvent(data);
	}

	auto WindowWin::registerNativeEvent(const WindowInitData& data) -> void
	{
		glfwSetWindowSizeCallback(nativeInterface, [](GLFWwindow* win, int32_t w, int32_t h) {
			app->getEventDispatcher().postEvent(std::make_unique<WindowResizeEvent>(w, h));
			app->onWindowResized(w, h);
		});

		glfwSetMouseButtonCallback(nativeInterface, [](GLFWwindow* window, int32_t btnId, int32_t state, int32_t mods) {
			double x;
			double y;
			glfwGetCursorPos(window, &x, &y);

			auto btn = -1;
			switch (btnId)
			{
			case GLFW_MOUSE_BUTTON_LEFT:btn = 0; break;
			case GLFW_MOUSE_BUTTON_MIDDLE:btn =1; break;
			case GLFW_MOUSE_BUTTON_RIGHT:btn = 2; break;
			}

			if (state == GLFW_PRESS || state == GLFW_REPEAT)
			{
				app->getEventDispatcher().postEvent(std::make_unique<MouseClickEvent>(btn,x, y));
			}
			if (state == GLFW_RELEASE)
			{
				app->getEventDispatcher().postEvent(std::make_unique<MouseReleaseEvent>(btn,x, y));
			}
			});


		glfwSetCursorPosCallback(nativeInterface, [](GLFWwindow*, double x, double y) {
			app->getEventDispatcher().postEvent(std::make_unique<MouseMoveEvent>(x, y));
			});

		glfwSetScrollCallback(nativeInterface, [](GLFWwindow* win, double xOffset, double yOffset) {
			double x;
			double y;
			glfwGetCursorPos(win, &x, &y);
			app->getEventDispatcher().postEvent(std::make_unique<MouseScrolledEvent>(xOffset, yOffset, 
				x, y));
			});

		glfwSetCharCallback(nativeInterface, [](GLFWwindow* window, unsigned int keycode) {
			app->getEventDispatcher().postEvent(std::make_unique<CharInputEvent>(KeyCode::Id(keycode),(char)keycode));
		});

		glfwSetKeyCallback(nativeInterface, [](GLFWwindow*, int32_t key, int32_t scancode, int32_t action, int32_t mods) {
			switch (action)
			{
			case GLFW_PRESS:
			{
				app->getEventDispatcher().postEvent(std::make_unique <KeyPressedEvent>(static_cast<KeyCode::Id>(key), 0));
				break;
			}
			case GLFW_RELEASE:
			{
				app->getEventDispatcher().postEvent(std::make_unique <KeyReleasedEvent>(static_cast<KeyCode::Id>(key)));
				break;
			}
			case GLFW_REPEAT:
			{
				app->getEventDispatcher().postEvent(std::make_unique <KeyPressedEvent>(static_cast<KeyCode::Id>(key), 1));
				break;
			}
			}
		});

	
	}


	auto WindowWin::getNativeWindow() -> void*
	{
		return glfwGetWin32Window(nativeInterface);
	}
};

