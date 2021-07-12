//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace Maple
{
	enum ShaderType
	{
		UNKNOWN,
		VERTEX_SHADER,
		FRAGMENT_SHADER,
		GEOMETRY_SHADER,
		COMPUTE_SHADER,
		TYPE_LENGTH
	};

	struct PushConstant
	{
		uint32_t size;
		ShaderType shaderStage;
		std::unique_ptr<uint8_t[]> data;
		uint32_t offset = 0;
	};
	
	class CommandBuffer;
	class Pipeline;

	class Shader
	{
	public:
		virtual ~Shader() {};
		virtual auto bind() const -> void = 0;
		virtual auto unbind() const -> void = 0;
		virtual auto bindPushConstants(CommandBuffer * cmdBuffer, Pipeline * pipeline) -> void = 0;
		virtual auto getHandle() const -> void* = 0;
		inline auto& getPushConstants() { return pushConstants; }
		auto getPushConstant(uint32_t index)->PushConstant*;
		inline auto& getFilePath() const { return path; }
		static auto create(const std::string & filePath)->std::shared_ptr<Shader>;

	protected:
		std::string path;
		std::vector<PushConstant> pushConstants;
	};
};