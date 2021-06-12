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
		VERTEX_SHADER,
		FRAGMENT_SHADER,
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

		static auto create(const std::vector<uint8_t>& vertex, const std::vector<uint8_t>& fragment)->std::shared_ptr<Shader>;

	protected:
		std::vector<PushConstant> pushConstants;
	};
};