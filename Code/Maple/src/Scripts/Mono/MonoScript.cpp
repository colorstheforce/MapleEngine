

#include "MonoScript.h"
#include "MonoSystem.h"
#include "MonoVirtualMachine.h"
#include "MapleMonoClass.h"
#include "MapleMonoObject.h"
#include "MonoComponent.h"
#include "Scene/Component/Transform.h"
#include "Scene/Entity/Entity.h"
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

		auto clazz = MonoVirtualMachine::get()->findClass("", name);
		if (clazz != nullptr) {
			scriptObject = clazz->createInstance(false);
			clazz->getAllFields();
		
			auto entity = component->getEntity();
			scriptObject->setValue(&component->getEntityId(), "_internal_entity_handle");
			scriptObject->setValue(entity.tryGetComponent<Transform>(), "_internal_entity_handle");
			scriptObject->construct();
		}
	}

	auto MonoScript::onStart( MonoSystem* system) -> void
	{
		if (scriptObject) {
			//system->callScriptStart(scriptInstance);
		}
	}

	auto MonoScript::onUpdate(float dt, MonoSystem* system) -> void
	{
		if (scriptObject) {
			//system->callScriptUpdate(scriptInstance,dt);
		}
	}

};