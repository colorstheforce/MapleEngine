//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "VulkanHelper.h"
#include "Engine/Interface/Shader.h"
#include "Engine/Interface/DescriptorSet.h"

namespace Maple
{
	class VulkanShader : public Shader
	{
	public:
		VulkanShader(const std::string & path);
		~VulkanShader();
		inline auto& getStageInfos() const { return stageInfos; }
        inline auto& getDescriptorLayoutInfo() const { return descriptorLayoutInfo; }
		inline auto getVertexInputStride() const { return vertexInputStride; }
		inline auto& getVertexInputAttributeDescription() const { return vertexInputAttributeDescriptions; };


		auto bind() const -> void override;
		auto unbind() const -> void override;
		auto bindPushConstants(CommandBuffer* cmdBuffer, Pipeline* pipeline) -> void override;
		auto getHandle() const -> void* override;

	private:

		static auto parseSource(const std::vector<std::string>& lines, std::unordered_map<ShaderType, std::string> &shaders) -> void;

		auto createShader(const std::vector<uint8_t>& data,ShaderType type)->VkShaderModule;
		std::unordered_map<ShaderType, VkShaderModule> shaderModules;
		std::vector<VkPipelineShaderStageCreateInfo> stageInfos;
	
		uint32_t vertexInputStride = 0;
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
		std::vector<DescriptorLayoutInfo> descriptorLayoutInfo;

	};
};