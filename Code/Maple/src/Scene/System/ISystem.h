//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Game Engine			                    //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Engine/Core.h"
namespace Maple 
{
	class Scene;

	class MAPLE_EXPORT ISystem
	{
	public:
		ISystem() = default;
		virtual ~ISystem() = default;
		virtual auto onInit() -> void = 0 ;
		virtual auto onUpdate(float dt, Scene* scene) -> void = 0;
		virtual auto onImGui() -> void = 0;
	};
};