//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <unordered_map>
#include <memory>

namespace Maple
{

	template <typename T>
	class Resources
	{
	public:
		virtual ~Resources()
		{
		}

		static auto get(const std::string & id) -> std::shared_ptr<T>
		{
			auto iter = cache.find(id);
			if (iter == cache.end())
			{
				iter = cache.emplace(id, std::shared_ptr<T>(new T(id)))
					.first;
			}
			return iter->second;
		}

		static auto remove(const std::string& id)
		{
			cache.erase(id);
		}

		static auto clear() -> void
		{
			cache.clear();
		}

	private:
#ifdef __GNUC__ 
		static std::map<std::string, std::shared_ptr<T>> cache;
#else
		static std::unordered_map<std::string, std::shared_ptr<T>> cache;
#endif

	};
	template <typename T>
#ifdef __GNUC__ 
	std::map<std::string, T> Cache<T>::cache;
#else
	std::unordered_map<std::string, std::shared_ptr<T>> Resources<T>::cache;
#endif


};