
#include "RenderPass.h"

#include "Engine/Vulkan/VulkanRenderPass.h"

namespace Maple 
{

	auto RenderPass::create(const RenderPassInfo& info) ->std::shared_ptr<RenderPass>
	{
		return std::make_shared<VulkanRenderPass>(info);
	}

};

