
#include "CommandBuffer.h"
#include "Engine/Vulkan/VulkanCommandBuffer.h"

namespace Maple 
{
	auto CommandBuffer::create(bool primary) -> std::shared_ptr<CommandBuffer>
	{
		return std::make_shared<VulkanCommandBuffer>(primary);
	}
};

