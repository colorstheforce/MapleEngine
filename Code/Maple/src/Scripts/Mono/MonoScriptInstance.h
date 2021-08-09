
#pragma once

#include "Mono.h"
#include "Others/Console.h"

namespace Maple 
{
	struct MonoScriptInstance 
	{
		MonoAssembly* assembly = nullptr;
		MonoImage* image = nullptr;
		MonoClass* klass = nullptr;
		MonoObject* object = nullptr;
		MonoMethod* methodStart = nullptr;
		MonoMethod* methodUpdate = nullptr;

		template<class T>
		auto setValue(T* value, const std::string& name) -> bool
		{
			if (MonoClassField* field = mono_class_get_field_from_name(klass, name.c_str()))
			{
				mono_field_set_value(object, field, value);
				return true;
			}

			LOGE("Failed to set value for field {0}", name.c_str());
			return false;
		}

	};
};