#pragma once

#define MAPLE_ASSERT(condition, ...)								\
	{																\
		if(!(condition))											\
		{															\
			LOGE("Assertion Failed : {0}", __VA_ARGS__);			\
			__debugbreak(); 										\
		}															\
	}



#ifdef PLATFORM_WINDOWS
#pragma warning(disable : 4251)

#ifdef MAPLE_DYNAMIC
#ifdef MAPLE_ENGINE
#define MAPLE_EXPORT __declspec(dllexport)
#else
#define MAPLE_EXPORT __declspec(dllimport)
#endif
#else
#define MAPLE_EXPORT
#endif

#define MAPLE_HIDDEN

#else

#define MAPLE_EXPORT __attribute__((visibility("default")))
#define MAPLE_HIDDEN __attribute__((visibility("hidden")))

#endif
