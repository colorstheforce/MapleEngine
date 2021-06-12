//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 
////////////////////////////////////////////////////////////////////////////// 



#include "Pipeline.h"
#include "Engine/Vulkan/VulkanPipeline.h"

namespace Maple 
{
	auto Pipeline::create(const PipelineInfo& pipelineCreateInfo) -> std::shared_ptr<Pipeline>
	{
		return std::make_shared<VulkanPipeline>(pipelineCreateInfo);
	}
};

