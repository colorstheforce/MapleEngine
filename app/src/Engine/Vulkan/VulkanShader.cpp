//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "VulkanShader.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanPipeline.h"
#include "FileSystem/File.h"
#include "Others/StringUtils.h"
#include "Others/Console.h"
#include <spirv_cross.hpp>

namespace Maple
{
	uint32_t getStrideFromVulkanFormat(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_R8_SINT:
		case VK_FORMAT_R32_SINT:
			return sizeof(int);
		case VK_FORMAT_R32_SFLOAT:
			return sizeof(float);
		case VK_FORMAT_R32G32_SFLOAT:
			return sizeof(glm::vec2);
		case VK_FORMAT_R32G32B32_SFLOAT:
			return sizeof(glm::vec3);
		case VK_FORMAT_R32G32B32A32_SFLOAT:
			return sizeof(glm::vec4);
		case VK_FORMAT_R32G32_SINT:
			return sizeof(glm::ivec2);
		case VK_FORMAT_R32G32B32_SINT:
			return sizeof(glm::ivec3);
		case VK_FORMAT_R32G32B32A32_SINT:
			return sizeof(glm::ivec4);
		case VK_FORMAT_R32G32_UINT:
			return sizeof(glm::ivec2);
		case VK_FORMAT_R32G32B32_UINT:
			return sizeof(glm::ivec3);
		case VK_FORMAT_R32G32B32A32_UINT:
			return sizeof(glm::ivec4); 
		default:
			LOGE("Unsupported Vulkan Format {0} , {1} : {2}", format,__FUNCTION__,__LINE__);
			return 0;
		}

		return 0;
	}

	auto getShaderTypeByName(const std::string& name) -> ShaderType 
	{
		if (StringUtils::contains(name, "Vertex")) {
			return VERTEX_SHADER;
		}

		if (StringUtils::contains(name, "Fragment")) {
			return FRAGMENT_SHADER;
		}

		if (StringUtils::contains(name, "Geometry")) {
			return GEOMETRY_SHADER;
		}
		if (StringUtils::contains(name, "Compute")) {
			return COMPUTE_SHADER;
		}
		return UNKNOWN;
	}


	VkFormat getVulkanFormat(const spirv_cross::SPIRType type)
	{
		VkFormat uintTypes[] =
		{
			VK_FORMAT_R32_UINT, 
			VK_FORMAT_R32G32_UINT, 
			VK_FORMAT_R32G32B32_UINT, 
			VK_FORMAT_R32G32B32A32_UINT
		};

		VkFormat intTypes[] =
		{
			VK_FORMAT_R32_SINT, 
			VK_FORMAT_R32G32_SINT, 
			VK_FORMAT_R32G32B32_SINT,
			VK_FORMAT_R32G32B32A32_SINT
		};

		VkFormat floatTypes[] =
		{
			VK_FORMAT_R32_SFLOAT, 
			VK_FORMAT_R32G32_SFLOAT, 
			VK_FORMAT_R32G32B32_SFLOAT,
			VK_FORMAT_R32G32B32A32_SFLOAT
		};

		VkFormat doubleTypes[] =
		{
			VK_FORMAT_R64_SFLOAT,
			VK_FORMAT_R64G64_SFLOAT,
			VK_FORMAT_R64G64B64_SFLOAT,
			VK_FORMAT_R64G64B64A64_SFLOAT,
		};

		switch (type.basetype)
		{
		case spirv_cross::SPIRType::UInt:
			return uintTypes[type.vecsize - 1];
		case spirv_cross::SPIRType::Int:
			return intTypes[type.vecsize - 1];
		case spirv_cross::SPIRType::Float:
			return floatTypes[type.vecsize - 1];
		case spirv_cross::SPIRType::Double:
			return doubleTypes[type.vecsize - 1];
		default:
			LOGC("Cannot find VK_Format : {0}", type.basetype); return VK_FORMAT_R32G32B32A32_SFLOAT;
		}
	}

	VulkanShader::~VulkanShader()
	{
		for (auto [type, shaderModule] : shaderModules) {
			vkDestroyShaderModule(*VulkanDevice::get(), shaderModule, nullptr);
		}
	}

	auto VulkanShader::bind() const -> void 
	{
	}

	auto VulkanShader::unbind() const -> void 
	{
	}

	auto VulkanShader::bindPushConstants(CommandBuffer* cmdBuffer, Pipeline* pipeline) -> void 
	{
		uint32_t index = 0;
		for (auto& pc : pushConstants)
		{
			vkCmdPushConstants(
				static_cast<VulkanCommandBuffer&>(*cmdBuffer),
				static_cast<VulkanPipeline*>(pipeline)->getPipelineLayout(),
				VkConverter::shaderTypeToVK(pc.shaderStage), index, pc.size, pc.data.get());
		}
	}

	auto VulkanShader::getHandle() const -> void* 
	{
		return nullptr;
	}

	VulkanShader::VulkanShader(const std::string& path)
	{

		auto buffer = File::read(path);
		std::string str(buffer.begin(), buffer.end());


		std::vector<std::string> lines;
		StringUtils::split(str,"\n", lines);
		std::unordered_map<ShaderType, std::string> sources;
		parseSource(lines, sources);

		for (auto & s : sources)
		{
			shaderModules[s.first] =   createShader(File::read(s.second), s.first);
		}

		for (auto [type, shaderModule] : shaderModules)
		{
			auto& info = stageInfos.emplace_back();
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			info.stage = VkConverter::shaderTypeToVK(type);
			info.module = shaderModule;
			info.pName = "main";
		}
	}


	auto VulkanShader::parseSource(const std::vector<std::string>& lines, std::unordered_map<ShaderType, std::string>& shaders) -> void
	{
		for (uint32_t i = 0; i < lines.size(); i++)
		{
			std::string str = lines[i];
			if (StringUtils::startWith(str,"#")) {
				auto path = StringUtils::split(str, " ");

				auto type = getShaderTypeByName(path[0]);
				if (type != UNKNOWN) {
					StringUtils::trim(path[1],"\r");
					shaders[type] = path[1];
				}
			}
		}
	}

	auto VulkanShader::createShader(const std::vector<uint8_t>& code, ShaderType shaderType) -> VkShaderModule
	{
		
		LOGI("Processing shader......................");

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		VkShaderModule shaderModule;
		if (vkCreateShaderModule(*VulkanDevice::get(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		std::vector<uint32_t> spv(createInfo.pCode, createInfo.pCode + code.size() / sizeof(uint32_t));

		spirv_cross::Compiler comp(spv);
		spirv_cross::ShaderResources resources = comp.get_shader_resources();

		if (shaderType == VERTEX_SHADER)
		{
			vertexInputStride = 0;
			for (const auto & resource : resources.stage_inputs)
			{
				auto & inputType = comp.get_type(resource.type_id);
				VkVertexInputAttributeDescription & description = vertexInputAttributeDescriptions.emplace_back();
				description.binding = comp.get_decoration(resource.id, spv::DecorationBinding);
				description.location = comp.get_decoration(resource.id, spv::DecorationLocation);
				description.offset = vertexInputStride;
				description.format = getVulkanFormat(inputType);
				vertexInputStride += getStrideFromVulkanFormat(description.format);
			}
		}

#define addLayout(BUFFER,DESCRIPTORTYPE)																\
			for (auto & uniform : BUFFER)																\
			{																							\
				auto set = comp.get_decoration(uniform.id, spv::DecorationDescriptorSet);				\
				auto binding = comp.get_decoration(uniform.id, spv::DecorationBinding);					\
				auto& type = comp.get_type(uniform.type_id);											\
				LOGV(###DESCRIPTORTYPE" {0} at set = {1}, binding = {2}", uniform.name, set, binding);	\
				auto& layout = descriptorLayoutInfo.emplace_back();										\
				layout.type = DESCRIPTORTYPE;															\
				layout.stage = shaderType;																\
				layout.setId = set;																		\
				layout.binding = binding;																\
				layout.count = type.array.size() ? uint32_t(type.array[0]) : 1;							\
			}


		addLayout(resources.uniform_buffers, DescriptorType::UNIFORM_BUFFER);
		addLayout(resources.sampled_images, DescriptorType::IMAGE_SAMPLER);

		for (auto& buffer : resources.push_constant_buffers)
		{
			auto set = comp.get_decoration(buffer.id, spv::DecorationDescriptorSet);
			auto binding = comp.get_decoration(buffer.id, spv::DecorationBinding);
			auto offset = comp.get_decoration(buffer.id, spv::DecorationOffset);
			auto& type = comp.get_type(buffer.type_id);
			auto ranges = comp.get_active_buffer_ranges(buffer.id);

			uint32_t size = 0;
			for (auto& range : ranges)
			{
				LOGV("Try to read PushConstant {0} offset {1}, size {2}", range.index, range.offset, range.range);
				size += uint32_t(range.range);
			}

			LOGV("Push Constant {0} at set = {1}, binding = {2}", buffer.name.c_str(), set, binding, type.array.size() ? uint32_t(type.array[0]) : 1);

			auto & back = pushConstants.emplace_back();
			back.size = size;
			back.shaderStage = shaderType;
			back.data = std::unique_ptr<uint8_t[]>(new uint8_t[size]);
		}

		
		return shaderModule;
	}
};