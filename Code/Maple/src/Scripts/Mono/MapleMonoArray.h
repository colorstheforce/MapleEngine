//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine									//
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Mono.h"
#include <string>
#include "MonoHelper.h"

namespace Maple
{
	class MAPLE_EXPORT MapleMonoArray
	{
	public:
		MapleMonoArray(MonoArray * existingArray);
		MapleMonoArray(MapleMonoClass & klass, uint32_t size);
		MapleMonoArray(MonoClass* klass, uint32_t size);

		auto size() const ->uint32_t;
		auto elementSize() const->uint32_t;
		

		inline auto getRawPtr() const { return monoArray; }


		template<class T>
		auto get(uint32_t idx)->T;
		template<class T>
		auto set(uint32_t idx, const T& value)-> void;

		auto setRaw(uint32_t idx, const uint8_t * value, uint32_t size, uint32_t count = 1);

		template<class T>
		inline auto getRaw(uint32_t idx = 0)
		{
			return (T*)getArrayAddr(monoArray, sizeof(T), idx);
		}


		inline auto getRaw(uint32_t index, uint32_t size) {
			return getArrayAddr(monoArray, size, index);
		}

		static auto getArrayAddr(MonoArray* array, uint32_t size, uint32_t idx) ->uint8_t*;

		template<class T>
		static auto create(uint32_t size)->MapleMonoArray;

		static auto getElementClass(MonoClass* arrayClass)->MonoClass*;

		static auto getRank(MonoClass* arrayClass)->uint32_t;

		static auto buildArrayClass(MonoClass* elementClass, uint32_t rank)->MonoClass*;

		static auto setArrayVal(MonoArray* array, uint32_t idx, const uint8_t* value, uint32_t size, uint32_t count = 1) -> void;


	private:
		MonoArray* monoArray = nullptr;
	};



	namespace Detail
	{
		// A layer of indirection for all methods specialized by MapleMonoArray. */
		template<class T>
		auto MapleMonoArray_get(MonoArray* array, uint32_t idx) -> T
		{
			return *(T*)MapleMonoArray::getArrayAddr(array, sizeof(T), idx);
		}

	
		template<class T>
		auto MapleMonoArray_set(MonoArray* array, uint32_t idx, const T& value) -> void
		{
			MapleMonoArray::setArrayVal(array, idx, (uint8_t*)&value, sizeof(T));
		}

		template<>
		auto MapleMonoArray_get(MonoArray* array, uint32_t idx)->std::string;

		template<>
		auto MapleMonoArray_set<std::string>(MonoArray* array, uint32_t idx, const std::string& value)  -> void;

		template<>
		auto MapleMonoArray_set<std::nullptr_t>(MonoArray* array, uint32_t idx, const std::nullptr_t & value)  -> void;


		template<class T>
		inline auto MapleMonoArray_create(uint32_t size) -> MapleMonoArray
		{
			return MapleMonoArray(*T::getMetaData()->scriptClass, size);
		}

		template<>
		inline auto MapleMonoArray_create<uint8_t>(uint32_t size) -> MapleMonoArray
		{
			return MapleMonoArray(MonoHelper::getByteClass(), size);
		}

		template<>
		inline auto MapleMonoArray_create<int8_t>(uint32_t size) -> MapleMonoArray
		{
			return MapleMonoArray(MonoHelper::getSByteClass(), size);
		}

		template<>
		inline auto MapleMonoArray_create<char>(uint32_t size) -> MapleMonoArray
		{
			return MapleMonoArray(MonoHelper::getSByteClass(), size);
		}

		template<>
		inline auto MapleMonoArray_create<uint16_t>(uint32_t size) -> MapleMonoArray
		{
			return MapleMonoArray(MonoHelper::getUINT16Class(), size);
		}

		template<>
		inline auto MapleMonoArray_create<int16_t>(uint32_t size) -> MapleMonoArray
		{
			return MapleMonoArray(MonoHelper::getINT16Class(), size);
		}

		template<>
		inline auto MapleMonoArray_create<uint32_t>(uint32_t size) -> MapleMonoArray
		{
			return MapleMonoArray(MonoHelper::getUINT32Class(), size);
		}

		template<>
		inline auto MapleMonoArray_create<int32_t>(uint32_t size) -> MapleMonoArray
		{
			return MapleMonoArray(MonoHelper::getINT32Class(), size);
		}

		template<>
		inline auto MapleMonoArray_create<uint64_t>(uint32_t size) -> MapleMonoArray
		{
			return MapleMonoArray(MonoHelper::getUINT64Class(), size);
		}

		template<>
		inline auto MapleMonoArray_create<int64_t>(uint32_t size) -> MapleMonoArray
		{
			return MapleMonoArray(MonoHelper::getINT64Class(), size);
		}

		template<>
		inline auto MapleMonoArray_create<std::string>(uint32_t size) -> MapleMonoArray
		{
			return MapleMonoArray(MonoHelper::getStringClass(), size);
		}

		template<>
		inline auto MapleMonoArray_create<float>(uint32_t size) -> MapleMonoArray
		{
			return MapleMonoArray(MonoHelper::getFloatClass(), size);
		}

		template<>
		inline auto MapleMonoArray_create<double>(uint32_t size) -> MapleMonoArray
		{
			return MapleMonoArray(MonoHelper::getDoubleClass(), size);
		}

		template<>
		inline auto MapleMonoArray_create<bool>(uint32_t size) -> MapleMonoArray
		{
			return MapleMonoArray(MonoHelper::getBoolClass(), size);
		}

		template<>
		inline auto MapleMonoArray_create<MonoObject*>(uint32_t size) -> MapleMonoArray
		{
			return MapleMonoArray(MonoHelper::getObjectClass(), size);
		}
	}


	template<class T>
	auto MapleMonoArray::get(uint32_t idx) -> T
	{
		return Detail::MapleMonoArray_get<T>(mInternal, idx);
	}

	template<class T>
	auto MapleMonoArray::set(uint32_t idx, const T& value) -> void
	{
		Detail::MapleMonoArray_set<T>(mInternal, idx, value);
	}

	template<class T>
	auto MapleMonoArray::create(uint32_t size) -> MapleMonoArray
	{
		return Detail::MapleMonoArray_create<T>(size);
	}


};