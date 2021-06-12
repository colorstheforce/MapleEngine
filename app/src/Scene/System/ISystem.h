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
		virtual void onInit() = 0;
		virtual void onUpdate(float dt, Scene* scene) = 0;
		virtual void onImGui() = 0;
	};
};