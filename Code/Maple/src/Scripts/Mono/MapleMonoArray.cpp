

#include "MapleMonoArray.h"
#include "MonoVirtualMachine.h"
#include "MapleMonoClass.h"
#include <cassert>

namespace Maple
{

	MapleMonoArray::MapleMonoArray(MonoArray* existingArray)
		:monoArray(existingArray)
	{
	}

	MapleMonoArray::MapleMonoArray(MapleMonoClass& klass, uint32_t size)
	{
		monoArray = mono_array_new(MonoVirtualMachine::get()->getDomain(), klass.getInternalClass(), size);
	}

	MapleMonoArray::MapleMonoArray(MonoClass* klass, uint32_t size)
	{
		monoArray = mono_array_new(MonoVirtualMachine::get()->getDomain(), klass, size);
	}

	auto MapleMonoArray::size() const -> uint32_t
	{
		return mono_array_length(monoArray);
	}

	auto MapleMonoArray::elementSize() const -> uint32_t
	{
		auto arrayClass = mono_object_get_class((MonoObject*)(monoArray));
		auto elementClass = mono_class_get_element_class(arrayClass);

		return (uint32_t)mono_class_array_element_size(elementClass);
	}

	auto MapleMonoArray::setRaw(uint32_t idx, const uint8_t* value, uint32_t size, uint32_t count)
	{
		setArrayVal(monoArray, idx, value, size, count);
	}

	auto MapleMonoArray::getArrayAddr(MonoArray* array, uint32_t size, uint32_t idx) -> uint8_t*
	{
		return (uint8_t * )mono_array_addr_with_size(array, size, idx);;
	}

	auto MapleMonoArray::getElementClass(MonoClass* arrayClass) -> MonoClass*
	{
		return mono_class_get_element_class(arrayClass);
	}

	auto MapleMonoArray::getRank(MonoClass* arrayClass) -> uint32_t
	{
		return (uint32_t)mono_class_get_rank(arrayClass);
	}

	auto MapleMonoArray::buildArrayClass(MonoClass* elementClass, uint32_t rank) -> MonoClass*
	{
		return mono_array_class_get(elementClass, rank);
	}

	auto MapleMonoArray::setArrayVal(MonoArray* array, uint32_t idx, const uint8_t* value, uint32_t size, uint32_t count) -> void
	{
		MonoClass* arrayClass = mono_object_get_class((MonoObject*)(array));
		MonoClass* elementClass = mono_class_get_element_class(arrayClass);

		assert((uint32_t)mono_class_array_element_size(elementClass) == size);
		assert((idx + count) <= mono_array_length(array));

		if (mono_class_is_valuetype(elementClass))
			mono_value_copy_array(array, idx, (void*)value, count);
		else
		{
			uint8_t* dest = getArrayAddr(array, size, idx);
			mono_gc_wbarrier_arrayref_copy(dest, (void*)value, count);
		}
	}


	namespace Detail
	{
		template<>
		auto MapleMonoArray_get<std::string>(MonoArray* array, uint32_t idx) -> std::string
		{
			return MonoHelper::monoToString(mono_array_get(array, MonoString*, idx));
		}

		template<>
		auto MapleMonoArray_set<std::string>(MonoArray* array, uint32_t idx, const std::string& value) -> void
		{
			MonoString* monoString = MonoHelper::stringToMono(value);
			mono_array_setref(array, idx, monoString);
		}


		template<>
		auto MapleMonoArray_set<std::nullptr_t>(MonoArray* array, uint32_t idx, const std::nullptr_t& value) -> void
		{
			void** item = (void**)MapleMonoArray::getArrayAddr(array, sizeof(void*), idx);
			*item = nullptr;
		}
	}


};
