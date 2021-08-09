
#include "MonoScriptInstance.h"
#include "MonoScript.h"
#include "MonoSystem.h"
#include "Others/StringUtils.h"
#include "IconsMaterialDesignIcons.h"

namespace Maple
{

	MonoScript::MonoScript(const std::string& name, MonoComponent* component, MonoSystem* system):
		component(component), name(name)
	{
		className = StringUtils::getFileNameWithoutExtension(name);
		classNameInEditor = "\t" + className;
		classNameInEditor = ICON_MDI_LANGUAGE_CSHARP + classNameInEditor;
		auto id = system->load(name, component);
		scriptInstance = system->getScript(id);
	}

	auto MonoScript::onStart( MonoSystem* system) -> void
	{
		if (scriptInstance) {
			system->callScriptStart(scriptInstance);
		}
	}

	auto MonoScript::onUpdate(float dt, MonoSystem* system) -> void
	{
		if (scriptInstance) {
			system->callScriptUpdate(scriptInstance,dt);
		}
	}

};