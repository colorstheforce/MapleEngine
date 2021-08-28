

#include "MapleMonoMethod.h"
#include "MapleMonoClass.h"
#include "Others/Console.h"
#include "MonoHelper.h"
#include "MonoVirtualMachine.h"
#include "MapleMonoField.h"

namespace Maple
{
	MapleMonoMethod::MapleMonoMethod(MonoMethod* method)
		:method(method)
	{

	}
	auto MapleMonoMethod::cacheSignature() const -> void
	{
		MonoMethodSignature* methodSignature = mono_method_signature(method);

		MonoType* returnType = mono_signature_get_return_type(methodSignature);
		if (returnType != nullptr)
		{
			auto returnClass = mono_class_from_mono_type(returnType);
			if (returnClass != nullptr)
				cachedReturnType = MonoVirtualMachine::get()->findClass(returnClass);
		}

		cachedNumParameters = (uint32_t)mono_signature_get_param_count(methodSignature);
		if (!cachedParameters.empty())
		{
			cachedParameters.clear();
		}

		if (cachedNumParameters > 0)
		{
			cachedParameters.resize(cachedNumParameters);

			void* iter = nullptr;
			for (auto i = 0; i < cachedNumParameters; i++)
			{
				auto curParamType = mono_signature_get_params(methodSignature, &iter);
				auto rawClass = mono_class_from_mono_type(curParamType);
				cachedParameters[i] = MonoVirtualMachine::get()->findClass(rawClass);
			}
		}

		_static = !mono_signature_is_instance(methodSignature);
		hasCachedSignature = true;
	}

	MapleMonoMethod::~MapleMonoMethod()
	{
		cachedParameters.clear();
	}
	auto MapleMonoMethod::invoke(MonoObject* instance, void** params) -> MonoObject*
	{
		MonoObject* exception = nullptr;
		auto retVal = mono_runtime_invoke(method, instance, params, &exception);

		MonoHelper::throwIfException(exception);
		return retVal;
	}
	auto MapleMonoMethod::invokeVirtual(MonoObject* instance, void** params) -> MonoObject*
	{
		auto virtualMethod = mono_object_get_virtual_method(instance, method);

		MonoObject* exception = nullptr;
		auto retVal = mono_runtime_invoke(virtualMethod, instance, params, &exception);

		MonoHelper::throwIfException(exception);
		return retVal;
	}
	auto MapleMonoMethod::getThunk() const -> void*
	{
		return mono_method_get_unmanaged_thunk(method);
	}
	auto MapleMonoMethod::getName() const -> std::string
	{
		return mono_method_get_name(method);
	}
	auto MapleMonoMethod::getReturnType() const -> std::shared_ptr<MapleMonoClass>
	{
		if (!hasCachedSignature)
			cacheSignature();

		return cachedReturnType;
	}
	auto MapleMonoMethod::getNumParameters() const -> uint32_t
	{
		if (!hasCachedSignature)
			cacheSignature();

		return cachedNumParameters;
	}
	auto MapleMonoMethod::getParameterType(uint32_t paramIdx) const -> std::shared_ptr<MapleMonoClass>
	{
		if(!hasCachedSignature)
			cacheSignature();

		if (paramIdx >= cachedNumParameters)
			LOGE("Parameter index out of range. Valid range is [0, {0}]", cachedNumParameters - 1);

		return cachedParameters[paramIdx];
	}
	auto MapleMonoMethod::isStatic() const -> bool
	{
		if (!hasCachedSignature)
			cacheSignature();
		return _static;
	}
	auto MapleMonoMethod::hasAttribute(std::shared_ptr<MapleMonoClass> monoClass) const -> bool
	{
		MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_method(method);
		if (attrInfo == nullptr)
			return false;
		bool hasAttr = mono_custom_attrs_has_attr(attrInfo, monoClass->getInternalClass()) != 0;
		mono_custom_attrs_free(attrInfo);
		return hasAttr;
	}
	auto MapleMonoMethod::getAttribute(std::shared_ptr<MapleMonoClass> monoClass) const -> MonoObject*
	{
		MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_method(method);
		if (attrInfo == nullptr)
			return nullptr;

		MonoObject* foundAttr = nullptr;
		if (mono_custom_attrs_has_attr(attrInfo, monoClass->getInternalClass()))
			foundAttr = mono_custom_attrs_get_attr(attrInfo, monoClass->getInternalClass());

		mono_custom_attrs_free(attrInfo);
		return foundAttr;
	}
	auto MapleMonoMethod::getVisibility() -> MonoMemberVisibility
	{
		uint32_t flags = mono_method_get_flags(method, nullptr) & MONO_METHOD_ATTR_ACCESS_MASK;

		if (flags == MONO_METHOD_ATTR_PRIVATE)
			return MonoMemberVisibility::Private;
		else if (flags == MONO_METHOD_ATTR_FAM_AND_ASSEM)
			return MonoMemberVisibility::ProtectedInternal;
		else if (flags == MONO_METHOD_ATTR_ASSEM)
			return MonoMemberVisibility::Internal;
		else if (flags == MONO_METHOD_ATTR_FAMILY)
			return MonoMemberVisibility::Protected;
		else if (flags == MONO_METHOD_ATTR_PUBLIC)
			return MonoMemberVisibility::Public;

		assert(false);
		return MonoMemberVisibility::Private;
	}

};