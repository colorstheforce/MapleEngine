#pragma once

#include "Others/Console.h"

#define MAPLE_ASSERT(condition, ...)								\
	{																\
		if(!(condition))											\
		{															\
			LOGE("Assertion Failed : {0}", __VA_ARGS__);			\
			__debugbreak(); 										\
		}															\
	}
