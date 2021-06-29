//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Game Engine			                    //
//////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Maple 
{
	class Scene;
	class ISystem
	{
	public:
		ISystem() = default;
		virtual ~ISystem() = default;
		virtual auto onInit() -> void = 0 ;
		virtual auto onUpdate(float dt, Scene* scene) -> void = 0;
		virtual auto onImGui() -> void = 0;
	};
};