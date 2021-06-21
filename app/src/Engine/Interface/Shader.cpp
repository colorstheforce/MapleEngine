//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "Shader.h"
#include "Engine/Vulkan/VulkanShader.h"
#include "Resources/ShaderResource.h"

namespace Maple 
{
	auto Shader::getPushConstant(uint32_t index) -> PushConstant*
	{
		if (index >= pushConstants.size()) {
			return nullptr;
		}
		return &pushConstants[index];
	}

	auto Shader::create(const std::string& filePath) ->std::shared_ptr<Shader>
	{
		if (auto shader = ShaderResource::tryGet(filePath)) {
			return shader;
		}
		auto shader= std::make_shared<VulkanShader>(filePath);
		ShaderResource::add(filePath, shader);
		return shader;
	}
};