//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "Shader.h"
#include "Engine/Vulkan/VulkanShader.h"

namespace Maple 
{
	auto Shader::getPushConstant(uint32_t index) -> PushConstant*
	{
		if (index >= pushConstants.size()) {
			return nullptr;
		}
		return &pushConstants[index];
	}

	auto Shader::create(const std::vector<uint8_t>& vertex, const std::vector<uint8_t>& fragment) ->std::shared_ptr<Shader>
	{
		return std::make_shared<VulkanShader>(vertex, fragment);
	}
};