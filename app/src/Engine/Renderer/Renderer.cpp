//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "Renderer.h"


#include "Engine/Vulkan/VulkanPipeline.h"
#include "Engine/Vulkan/VulkanCommandBuffer.h"
#include "Engine/Vulkan/VulkanDescriptorSet.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Interface/SwapChain.h"


namespace Maple 
{
	auto Renderer::bindDescriptorSets(Pipeline* pipeline, CommandBuffer* cmdBuffer, uint32_t dynamicOffset,const std::vector<std::shared_ptr<DescriptorSet>>& descriptorSets) -> void
	{
		uint32_t numDynamicDescriptorSets = 0;
		uint32_t numDesciptorSets = 0;
		VkDescriptorSet descriptorSetPool[16];
		for (auto descriptorSet : descriptorSets)
		{
			if (descriptorSet)
			{
				auto vkDesSet = std::static_pointer_cast<VulkanDescriptorSet>(descriptorSet);
				if (vkDesSet->isDynamic())
					numDynamicDescriptorSets++;
				descriptorSetPool[numDesciptorSets] = *vkDesSet;
				numDesciptorSets++;
			}
		}
		vkCmdBindDescriptorSets(*static_cast<VulkanCommandBuffer*>(cmdBuffer), VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<VulkanPipeline*>(pipeline)->getPipelineLayout(), 0, numDesciptorSets, descriptorSetPool, numDynamicDescriptorSets, &dynamicOffset);
	}

	auto Renderer::drawIndexed(CommandBuffer* commandBuffer, DrawType type, uint32_t count, uint32_t start) -> void
	{
		vkCmdDrawIndexed(*static_cast<VulkanCommandBuffer*>(commandBuffer), count, 1, 0, 0, 0);
	}

	auto Renderer::getCommandBuffer() -> CommandBuffer*
	{
		return VulkanContext::get()->getSwapChain()->getCurrentCommandBuffer();
	}

	auto Renderer::setRenderManager(RenderManager* manager) -> void
	{
		this->manager = manager;
	}

};
