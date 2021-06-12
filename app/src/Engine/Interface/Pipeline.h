//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 
////////////////////////////////////////////////////////////////////////////// 


#pragma once
#include <memory>
#include "Engine/Renderer/RenderParam.h"
#include "Engine/Interface/BufferLayout.h"

namespace Maple
{
	class CommandBuffer;
	class DescriptorSet;
	class Shader;
	class RenderPass;

	struct PipelineInfo
	{
		std::shared_ptr<RenderPass> renderPass;
		std::shared_ptr<Shader> shader;

		BufferLayout vertexBufferLayout;

		CullMode cullMode = CullMode::BACK;
		PolygonMode polygonMode = PolygonMode::FILL;
		DrawType drawType = DrawType::TRIANGLE;

		//will be used in future
		bool transparencyEnabled;
		bool depthBiasEnabled;
		bool depthWriteEnable = true;
		bool depthTestEnable = true;
	};

	class Pipeline 
	{
	public:
		virtual auto bind(CommandBuffer* buffer) -> void = 0;
		virtual auto getDescriptorSet()->std::shared_ptr<DescriptorSet> = 0;
		virtual auto getShader()->std::shared_ptr<Shader> = 0;
		static auto create(const PipelineInfo& pipelineCreateInfo) -> std::shared_ptr<Pipeline>;
	};
};