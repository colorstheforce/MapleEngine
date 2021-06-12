
#include "Engine/Vulkan/VulkanSwapChain.h"

namespace Maple 
{
	auto SwapChain::create() ->std::shared_ptr<SwapChain>
	{
		return std::make_shared<VulkanSwapChain>();
	}

	auto SwapChain::resize(uint32_t width, uint32_t height) -> void
	{
		
	}

};

