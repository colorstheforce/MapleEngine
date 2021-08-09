
#include "FrameBuffer.h"
#include "Engine/Vulkan/VulkanFrameBuffer.h"
#include "Others/HashCode.h"

namespace Maple 
{
	std::unordered_map<size_t, std::shared_ptr<FrameBuffer>> FrameBuffer::frameBufferCache;

	auto FrameBuffer::create(const FrameBufferInfo& info) ->std::shared_ptr<FrameBuffer>
	{
		/*auto hash = info.hashCode();
		auto found = frameBufferCache.find(hash);
		if (found != frameBufferCache.end() && found->second)
		{
			//resize issue, it could be the hashCode problem. 
			return found->second;
		}
		auto framebuffer = std::make_shared<VulkanFrameBuffer>(info);
		frameBufferCache[hash] = framebuffer;*/
		return std::make_shared<VulkanFrameBuffer>(info);;
	}


	auto FrameBufferInfo::hashCode() const-> size_t
	{
		size_t hash = 0;
		HashCode::hashCode(hash, attachments.size(), width, height, layer, renderPass.get(), screenFBO);

		for (uint32_t i = 0; i < types.size(); i++)
		{
			HashCode::hashCode(hash, types[i], attachments[i].get());
		}
		return hash;
	}

};

