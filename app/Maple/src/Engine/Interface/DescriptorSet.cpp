//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "DescriptorSet.h"

#include "Engine/Vulkan/VulkanDescriptorSet.h"

namespace Maple 
{
	auto DescriptorSet::create(const DescriptorInfo& info) ->std::shared_ptr<DescriptorSet>
	{
		return std::make_shared<VulkanDescriptorSet>(info);
	}
};