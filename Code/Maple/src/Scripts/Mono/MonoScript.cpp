

#include "MonoScript.h"
#include "MonoSystem.h"
#include "MonoVirtualMachine.h"
#include "MapleMonoClass.h"
#include "MapleMonoObject.h"
#include "MapleMonoMethod.h"
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

		loadFunction();
	}

	auto MonoScript::onStart( MonoSystem* system) -> void
	{
		if (startFunc) {
			startFunc->invokeVirtual(scriptObject->getRawPtr());
		}
	}

	auto MonoScript::onUpdate(float dt, MonoSystem* system) -> void
	{
		if (updateFunc) {
			updateFunc->invokeVirtual(scriptObject->getRawPtr(), dt);
		}
	}

	MonoScript::~MonoScript()
	{
		if (destoryFunc) {
			destoryFunc->invokeVirtual(scriptObject->getRawPtr());
		}
	}

	auto MonoScript::loadFunction() -> void
	{
		auto clazz = MonoVirtualMachine::get()->findClass("", className);
		if (clazz != nullptr) {
			scriptObject = clazz->createInstance(false);
			clazz->getAllMethods();
			startFunc = clazz->getMethodExact("OnStart", "");
			updateFunc = clazz->getMethodExact("OnUpdate", "single");
			destoryFunc = clazz->getMethodExact("OnDestory", "");
			auto entity = component->getEntity();
			scriptObject->setValue(&component->getEntityId(), "_internal_entity_handle");
			scriptObject->setValue(entity.tryGetComponent<Transform>(), "_internal_entity_handle");
			scriptObject->construct();
		}
	}

};