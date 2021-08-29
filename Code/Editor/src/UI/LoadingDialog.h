//////////////////////////////////////////////////////////////////////////////
// This file is part of the Vulkan Assignment								// 
// Copyright ?2020-2022 Prime Zeng                                          // 
////////////////////////////////////////////////////////////////////////////// 

#pragma once
#include <string>

namespace Maple 
{
	class LoadingDialog 
	{
	public:
		auto show(const std::string & name = "LoadingDialog")-> void;
		auto close() -> void;
		auto onImGui() -> void;
	private:
		bool active = false;
		std::string name;
	};
};
