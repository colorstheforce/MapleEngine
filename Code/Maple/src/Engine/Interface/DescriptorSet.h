//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "Engine/Core.h"

namespace Maple
{

	enum ShaderType;
	enum class TextureType;

	class Shader;
	class Pipeline;
	class UniformBuffer;
	class Texture;


	enum class DescriptorType
	{
		UNIFORM_BUFFER,
		UNIFORM_BUFFER_DYNAMIC,
		IMAGE_SAMPLER
	};


	struct ImageInfo
	{
		std::vector<std::shared_ptr<Texture>> textures;
		int32_t binding;
		std::string name;
		TextureType type;
	};

	struct BufferInfo
	{
		std::shared_ptr<UniformBuffer> buffer;
		uint32_t offset;
		uint32_t size;
		int32_t binding = 0;
		std::string name = "";
		DescriptorType type;
		ShaderType shaderType;
	};

	struct DescriptorInfo
	{
		Pipeline* pipeline = nullptr;
		uint32_t layoutIndex;
		std::shared_ptr<Shader> shader;
		uint32_t count = 1;
	};

	enum class Format
	{
		R32G32B32A32_UINT,
		R32G32B32_UINT,
		R32G32_UINT,
		R32_UINT,
		R8_UINT,
		R32G32B32A32_INT,
		R32G32B32_INT,
		R32G32_INT,
		R32_INT,
		R32G32B32A32_FLOAT,
		R32G32B32_FLOAT,
		R32G32_FLOAT,
		R32_FLOAT
	};


	//it will be generated from SprivCross lib
	struct DescriptorLayoutInfo
	{
		DescriptorType type;
		ShaderType stage;
		uint32_t binding = 0;
		uint32_t setId = 0;
		uint32_t count = 1;
	};



	class MAPLE_EXPORT DescriptorSet
	{
	public:
		virtual ~DescriptorSet() {};
		virtual auto update(const std::vector<ImageInfo>& imageInfos, const std::vector<BufferInfo>& bufferInfos) -> void = 0;
		virtual auto update(const std::vector<ImageInfo>& imageInfos) -> void = 0;
		virtual auto update(const std::vector<BufferInfo>& bufferInfos) -> void = 0;
		
		static auto create(const DescriptorInfo& info)->std::shared_ptr<DescriptorSet>;

	};
};