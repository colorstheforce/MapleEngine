
#include "MapleMonoField.h"
#include "MapleMonoClass.h"
#include "MonoVirtualMachine.h"
#include <cassert>


namespace Maple
{

	auto MapleMonoField::getType() ->std::shared_ptr<MapleMonoClass>
	{
		if (fieldType != nullptr)
			return fieldType;

		MonoType* monoType = mono_field_get_type(monoField);
		MonoClass* fieldClass = mono_class_from_mono_type(monoType);
		if (fieldClass == nullptr)
			return nullptr;

		fieldType = MonoVirtualMachine::get()->findClass(fieldClass);

		return fieldType;
	}

	auto MapleMonoField::get(MonoObject* instance, void* outValue) -> void
	{
		mono_field_get_value(instance, monoField, outValue);
	}

	auto MapleMonoField::set(MonoObject* instance, void* value) -> void
	{
		mono_field_set_value(instance, monoField, value);
	}

	auto MapleMonoField::hasAttribute(std::shared_ptr<MapleMonoClass> monoClass) -> bool
	{
		MonoClass* parentClass = mono_field_get_parent(monoField);
		MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_field(parentClass, monoField);
		if (attrInfo == nullptr)
			return false;

		bool hasAttr = mono_custom_attrs_has_attr(attrInfo, monoClass->getInternalClass()) != 0;

		mono_custom_attrs_free(attrInfo);

		return hasAttr;
	}

	auto MapleMonoField::getAttribute(std::shared_ptr<MapleMonoClass> monoClass) -> MonoObject*
	{
		auto parentClass = mono_field_get_parent(monoField);
		MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_field(parentClass, monoField);
		if (attrInfo == nullptr)
			return nullptr;

		MonoObject* foundAttr = nullptr;
		if (mono_custom_attrs_has_attr(attrInfo, monoClass->getInternalClass()))
			foundAttr = mono_custom_attrs_get_attr(attrInfo, monoClass->getInternalClass());

		mono_custom_attrs_free(attrInfo);
		return foundAttr;
	}

	auto MapleMonoField::isStatic() -> bool
	{
		uint32_t flags = mono_field_get_flags(monoField);
		return (flags & MONO_FIELD_ATTR_STATIC) != 0;
	}

	auto MapleMonoField::getVisibility() -> MonoMemberVisibility
	{

		uint32_t flags = mono_field_get_flags(monoField) & MONO_FIELD_ATTR_FIELD_ACCESS_MASK;

		if (flags == MONO_FIELD_ATTR_PRIVATE)
			return MonoMemberVisibility::Private;
		else if (flags == MONO_FIELD_ATTR_FAM_AND_ASSEM)
			return MonoMemberVisibility::ProtectedInternal;
		else if (flags == MONO_FIELD_ATTR_ASSEMBLY)
			return MonoMemberVisibility::Internal;
		else if (flags == MONO_FIELD_ATTR_FAMILY)
			return MonoMemberVisibility::Protected;
		else if (flags == MONO_FIELD_ATTR_PUBLIC)
			return MonoMemberVisibility::Public;

		assert(false);
		return MonoMemberVisibility::Private;
	}

	auto MapleMonoField::getBoxed(MonoObject* instance) -> MonoObject*
	{
		return mono_field_get_value_object(MonoVirtualMachine::get()->getDomain(), monoField, instance);
	}

	MapleMonoField::MapleMonoField(MonoClassField* field)
		:monoField(field)
	{
		name = mono_field_get_name(field);
	}

};