#include "MapleMonoProperty.h"
#include "MapleMonoClass.h"
#include "MapleMonoField.h"
#include "MapleMonoMethod.h"
#include "MonoVirtualMachine.h"

namespace Maple
{
	auto MapleMonoProperty::get(MonoObject* instance) const -> MonoObject*
	{
		if (getMethod == nullptr)
			return nullptr;
		return mono_runtime_invoke(getMethod, instance, nullptr, nullptr);
	}

	auto MapleMonoProperty::set(MonoObject* instance, void* value) const -> void
	{
		if (setMethod == nullptr)
			return;

		void* args[1];
		args[0] = value;
		mono_runtime_invoke(setMethod, instance, args, nullptr);
	}

	auto MapleMonoProperty::getIndexed(MonoObject* instance, uint32_t index) const -> MonoObject*
	{
		void* args[1];
		args[0] = &index;
		return mono_runtime_invoke(getMethod, instance, args, nullptr);
	}

	auto MapleMonoProperty::setIndexed(MonoObject* instance, uint32_t index, void* value) const -> void
	{
		void* args[2];
		args[0] = &index;
		args[1] = value;
		mono_runtime_invoke(setMethod, instance, args, nullptr);
	}

	auto MapleMonoProperty::isIndexed() const -> bool
	{
		if (!fullyInitialized)
			initializeDeferred();
		return indexed;
	}

	auto MapleMonoProperty::getReturnType() const ->std::shared_ptr<MapleMonoClass>
	{
		if (!fullyInitialized)
			initializeDeferred();
		return returnType;
	}
	auto MapleMonoProperty::hasAttribute(MapleMonoClass* monoClass) -> bool
	{

		MonoClass* parentClass = mono_property_get_parent(monoProperty);
		MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_property(parentClass, monoProperty);
		if (attrInfo == nullptr)
			return false;

		bool hasAttr = mono_custom_attrs_has_attr(attrInfo, monoClass->getInternalClass()) != 0;

		mono_custom_attrs_free(attrInfo);

		return hasAttr;
	}
	auto MapleMonoProperty::getAttribute(MapleMonoClass* monoClass) -> MonoObject*
	{
		MonoClass* parentClass = mono_property_get_parent(monoProperty);
		MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_property(parentClass, monoProperty);
		if (attrInfo == nullptr)
			return nullptr;

		MonoObject* foundAttr = nullptr;
		if (mono_custom_attrs_has_attr(attrInfo, monoClass->getInternalClass()))
			foundAttr = mono_custom_attrs_get_attr(attrInfo, monoClass->getInternalClass());

		mono_custom_attrs_free(attrInfo);
		return foundAttr;
	}
	auto MapleMonoProperty::getVisibility() -> MonoMemberVisibility
	{
		MonoMemberVisibility getterVisibility = MonoMemberVisibility::Public;
		if (getMethod)
		{
			MapleMonoMethod getterWrapper(getMethod);
			getterVisibility = getterWrapper.getVisibility();
		}

		MonoMemberVisibility setterVisibility = MonoMemberVisibility::Public;
		if (setMethod)
		{
			MapleMonoMethod setterWrapper(setMethod);
			setterVisibility = setterWrapper.getVisibility();
		}

		if (getterVisibility < setterVisibility)
			return getterVisibility;

		return setterVisibility;
	}

	MapleMonoProperty::MapleMonoProperty(MonoProperty* monoProp)
		:monoProperty(monoProp)
	{
		getMethod = mono_property_get_get_method(monoProp);
		setMethod = mono_property_get_set_method(monoProp);
		name = mono_property_get_name(monoProp);
	}

	auto MapleMonoProperty::initializeDeferred() const -> void
	{
		if (getMethod != nullptr)
		{
			MonoMethodSignature* signature = mono_method_signature(getMethod);

			MonoType* returnType = mono_signature_get_return_type(signature);
			if (returnType != nullptr)
			{
				auto returnClass = mono_class_from_mono_type(returnType);
				if (returnClass != nullptr)
					this->returnType = MonoVirtualMachine::get()->findClass(returnClass);
			}

			uint32_t numParams = mono_signature_get_param_count(signature);
			indexed = numParams == 1;
		}
		else if (setMethod != nullptr)
		{
			MonoMethodSignature* signature = mono_method_signature(setMethod);

			MonoType* returnType = mono_signature_get_return_type(signature);
			if (returnType != nullptr)
			{
				auto returnClass = mono_class_from_mono_type(returnType);
				if (returnClass != nullptr)
					this->returnType = MonoVirtualMachine::get()->findClass(returnClass);
			}

			uint32_t numParams = mono_signature_get_param_count(signature);
			indexed = numParams == 2;
		}

		fullyInitialized = true;
	}
};