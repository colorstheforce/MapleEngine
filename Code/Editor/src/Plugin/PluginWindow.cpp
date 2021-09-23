//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "PluginWindow.h"
#include "Application.h"
#include "Scripts/Mono/MonoVirtualMachine.h"
#include "Scripts/Mono/MapleMonoAssembly.h"
#include "Scripts/Mono/MapleMonoClass.h"
#include "Scripts/Mono/MapleMonoMethod.h"
namespace Maple
{
	PluginWindow::PluginWindow()
	{
		auto vm = Application::get()->getMonoVm();
		auto & classes = vm->getAssembly("MapleAssembly.dll")->getAllClasses();
		
		for (auto & clazz : classes)
		{
			auto & methods = clazz->getAllMethods();
			for (auto & method : methods)
			{
				if (method->isStatic())
				{
					if(method->getAttribute(vm->findClass("Editor", "MenuItem")))
					{
						method->invoke(nullptr);
					}
				
				}
				
			}
		}

		auto editorWindow = vm->findClass("Editor", "EditorWindow");

		if (auto method = editorWindow->getMethod("ImGui"); method != nullptr) 
		{
			imguiFuncs.emplace_back(method);
		}
	}

	auto PluginWindow::onImGui() -> void
	{
		for (auto & func : imguiFuncs)
		{
			func->invoke(nullptr);
		}
	}
};