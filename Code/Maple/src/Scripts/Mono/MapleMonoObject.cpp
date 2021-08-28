
#include "MapleMonoObject.h"
#include "MapleMonoClass.h"
#include "Others/Console.h"

namespace Maple 
{
	MapleMonoObject::MapleMonoObject(MonoObject* rawPtr, MapleMonoClass* clazz)
		:rawPtr(rawPtr), clazz(clazz)
	{

	}

	auto MapleMonoObject::setValue(void* value, const std::string& name) -> void
	{
		if (MonoClassField* field = mono_class_get_field_from_name(clazz->clazz, name.c_str()))
		{
			mono_field_set_value(rawPtr, field, value);
		}
		else 
		{
			LOGE("Failed to setValue to {0}",name);
		}
	}

	auto MapleMonoObject::construct() -> void
	{
		mono_runtime_object_init(rawPtr);
	}

};



