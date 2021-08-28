

#include "MapleMonoClass.h"
#include "MapleMonoMethod.h"
#include "MapleMonoObject.h"
#include "MapleMonoField.h"
#include "MapleMonoProperty.h"
#include "MonoVirtualMachine.h"
#include "MonoHelper.h"
#include "Others/Console.h"
#include "Others/HashCode.h"

namespace Maple 
{
	MapleMonoClass::MapleMonoClass(const std::string& nameSpace, const std::string& type, MonoClass* monoClass, const MapleMonoAssembly* parentAssembly)
		: parentAssembly(parentAssembly), clazz(monoClass), nameSpace(nameSpace), typeName(type), hasCachedFields(false)
		, hasCachedProperties(false), hasCachedMethods(false)

	{

	}

	MapleMonoClass::~MapleMonoClass()
	{
		methods.clear();
		fields.clear();
		properties.clear();
	}

	auto MapleMonoClass::getMethod(const std::string& name, uint32_t numParams) const -> std::shared_ptr<MapleMonoMethod>
	{
		MethodId mehodId(name, numParams);
		auto iterFind = methods.find(mehodId);
		if (iterFind != methods.end())
			return iterFind->second;

		auto method = mono_class_get_method_from_name(clazz, name.c_str(), (int32_t)numParams);
		if (method == nullptr)
			return nullptr;

		auto newMethod = std::make_shared<MapleMonoMethod>(method);
		methods[mehodId] = newMethod;
		return newMethod;
	}

	auto MapleMonoClass::getMethodExact(const std::string& name, const std::string& signature) const ->std::shared_ptr<MapleMonoMethod>
	{
		MethodId mehodId(name + "(" + signature + ")", 0);
		auto iterFind = methods.find(mehodId);
		if (iterFind != methods.end())
			return iterFind->second;

		MonoMethod* method;
		void* iter = nullptr;

		const char* rawName = name.c_str();
		const char* rawSig = signature.c_str();
		while ((method = mono_class_get_methods(clazz, &iter)))
		{
			if (strcmp(rawName, mono_method_get_name(method)) == 0)
			{
				const char* curSig = mono_signature_get_desc(mono_method_signature(method), false);
				if (strcmp(rawSig, curSig) == 0)
				{
					auto newMethod = std::make_shared<MapleMonoMethod>(method);
					methods[mehodId] = newMethod;
					return newMethod;
				}
			}
		}
		return nullptr;
	}

	auto MapleMonoClass::getField(const std::string& name) const -> std::shared_ptr<MapleMonoField>
	{
		auto iterFind = fields.find(name);
		if (iterFind != fields.end())
			return iterFind->second;

		auto field = mono_class_get_field_from_name(clazz, name.c_str());
		if (field == nullptr)
			return nullptr;

		auto newField = std::make_shared<MapleMonoField>(field);
		fields[name] = newField;
		return newField;
	}

	auto MapleMonoClass::getProperty(const std::string& name) const -> std::shared_ptr<MapleMonoProperty>
	{
		auto iterFind = properties.find(name);
		if (iterFind != properties.end())
			return iterFind->second;

		auto property = mono_class_get_property_from_name(clazz, name.c_str());
		if (property == nullptr)
			return nullptr;

		auto newProperty = std::make_shared<MapleMonoProperty>(property);
		properties[name] = newProperty;
		return newProperty;
	}

	auto MapleMonoClass::getAttribute(MapleMonoClass* monoClass) const -> MonoObject*
	{
		MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_class(clazz);
		if (attrInfo == nullptr)
			return nullptr;

		MonoObject* foundAttr = nullptr;
		if (mono_custom_attrs_has_attr(attrInfo, monoClass->getInternalClass()))
			foundAttr = mono_custom_attrs_get_attr(attrInfo, monoClass->getInternalClass());

		mono_custom_attrs_free(attrInfo);
		return foundAttr;
	}

	auto MapleMonoClass::getBaseClass() const ->std::shared_ptr<MapleMonoClass>
	{
		auto monoBase = mono_class_get_parent(clazz);
		if (monoBase == nullptr)
			return nullptr;

		std::string ns;
		std::string type;
		MonoHelper::getClassName(monoBase, ns, type);
		return MonoVirtualMachine::get()->findClass(ns, type);
	}



	auto MapleMonoClass::getAllFields() const -> const std::vector< std::shared_ptr<MapleMonoField>>&
	{
		if (hasCachedFields)
			return cachedFieldList;

		cachedFieldList.clear();

		void* iter = nullptr;
		MonoClassField* curClassField = mono_class_get_fields(clazz, &iter);
		while (curClassField != nullptr)
		{
			const char* fieldName = mono_field_get_name(curClassField);
			LOGI("{0}____{1}", fullName,fieldName);
			auto curField = getField(fieldName);
			cachedFieldList.emplace_back(curField);
			curClassField = mono_class_get_fields(clazz, &iter);
		}
		hasCachedFields = true;
		return cachedFieldList;
	}

	auto MapleMonoClass::getAllProperties() const -> const std::vector< std::shared_ptr<MapleMonoProperty>>&
	{
		if (hasCachedProperties)
			return cachedPropertyList;

		cachedPropertyList.clear();
		void* iter = nullptr;
		auto curClassProperty = mono_class_get_properties(clazz, &iter);
		while (curClassProperty != nullptr)
		{
			const char* propertyName = mono_property_get_name(curClassProperty);
			auto curProperty = getProperty(propertyName);
			cachedPropertyList.emplace_back(curProperty);
			curClassProperty = mono_class_get_properties(clazz, &iter);
		}
		hasCachedProperties = true;
		return cachedPropertyList;
	}

	auto MapleMonoClass::getAllMethods() const -> const std::vector< std::shared_ptr<MapleMonoMethod> >&
	{
		if (hasCachedMethods)
			return cachedMethodList;

		cachedMethodList.clear();

		void* iter = nullptr;
		auto curClassMethod = mono_class_get_methods(clazz, &iter);
		while (curClassMethod != nullptr)
		{
			auto sig = mono_method_signature(curClassMethod);
			const auto sigDesc = mono_signature_get_desc(sig, false);
			const auto methodName = mono_method_get_name(curClassMethod);

			auto curMethod = getMethodExact(methodName, sigDesc);
			cachedMethodList.push_back(curMethod);

			curClassMethod = mono_class_get_methods(clazz, &iter);
		}

		hasCachedMethods = true;
		return cachedMethodList;
	}

	auto MapleMonoClass::getAllAttributes() const -> std::vector<std::shared_ptr<MapleMonoClass>>
	{
		std::vector<std::shared_ptr<MapleMonoClass>> attributes;

		MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_class(clazz);
		if (attrInfo == nullptr)
			return attributes;

		for (auto i = 0; i < attrInfo->num_attrs; i++)
		{
			auto attribClass = mono_method_get_class(attrInfo->attrs[i].ctor);
			auto klass = MonoVirtualMachine::get()->findClass(attribClass);

			if (klass != nullptr)
				attributes.push_back(klass);
		}

		mono_custom_attrs_free(attrInfo);

		return attributes;
	}

	auto MapleMonoClass::hasAttribute(std::shared_ptr<MapleMonoClass> monoClass) const -> bool
	{
		MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_class(clazz);
		if (attrInfo == nullptr)
			return false;

		bool hasAttr = mono_custom_attrs_has_attr(attrInfo, monoClass->getInternalClass()) != 0;

		mono_custom_attrs_free(attrInfo);

		return hasAttr;
	}

	auto MapleMonoClass::hasField(const std::string& name) const -> bool
	{
		return mono_class_get_field_from_name(clazz, name.c_str()) != nullptr;
	}

	auto MapleMonoClass::isSubClassOf(const std::shared_ptr<MapleMonoClass> & monoClass) const -> bool
	{
		if (monoClass == nullptr)
			return false;
		return mono_class_is_subclass_of(clazz, monoClass->clazz, true) != 0;
	}

	auto MapleMonoClass::isInstanceOfType(MonoObject* object) const -> bool
	{
		if (object == nullptr)
			return false;

		auto monoClass = mono_object_get_class(object);
		return mono_class_is_subclass_of(monoClass, clazz, false) != 0;
	}

	auto MapleMonoClass::getInstanceSize() const -> uint32_t
	{
		uint32_t align = 0;
		if (mono_class_is_valuetype(clazz))
			return mono_class_value_size(clazz, &align);
		return mono_class_instance_size(clazz);
	}

	auto MapleMonoClass::invokeMethod(const std::string& name, MonoObject* instance, void** params, uint32_t numParams) -> MonoObject*
	{
		return getMethod(name, numParams)->invoke(instance, params);
	}

	auto MapleMonoClass::addInternalCall(const std::string& name, const void* method) -> void
	{
		auto fullMethodName = fullName + "::" + name;
		mono_add_internal_call(fullMethodName.c_str(), method);
	}

	auto MapleMonoClass::createInstance(bool construct) -> std::shared_ptr<MapleMonoObject>
	{
		auto obj = mono_object_new(MonoVirtualMachine::get()->getDomain(), clazz);

		if (construct)
			mono_runtime_object_init(obj);

		return std::make_shared<MapleMonoObject>(obj, this);
	}

	auto MapleMonoClass::createInstance(void** params, uint32_t numParams) -> std::shared_ptr<MapleMonoObject>
	{
		auto obj = mono_object_new(MonoVirtualMachine::get()->getDomain(), clazz);
		getMethod(".ctor", numParams)->invoke(obj, params);
		return std::make_shared<MapleMonoObject>(obj, this);
	}

	auto MapleMonoClass::createInstance(const std::string& ctorSignature, void** params) -> std::shared_ptr<MapleMonoObject>
	{
		MonoObject* obj = mono_object_new(MonoVirtualMachine::get()->getDomain(), clazz);
		getMethodExact(".ctor", ctorSignature)->invoke(obj, params);
		return std::make_shared<MapleMonoObject>(obj, this);
	}

	auto MapleMonoClass::construct(MonoObject* object) -> void
	{
		mono_runtime_object_init(object);
	}

	auto MapleMonoClass::MethodId::Hash::operator()(const MethodId& v) const -> size_t
	{
		size_t seed = 0;
		HashCode::hashCode(seed, v.name, v.numParams);
		return seed;
	}


	MapleMonoClass::MethodId::MethodId(const std::string& name, uint32_t numParams)
		:name(name), numParams(numParams)
	{

	}

};

