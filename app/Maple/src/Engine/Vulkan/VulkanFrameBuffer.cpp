//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 

////////////////////////////////////////////////////////////////////////////// 

#include "VulkanHelper.h"
#include "VulkanRenderPass.h"
#include "VulkanFrameBuffer.h"
#include "VulkanDevice.h"
#include "VulkanTexture.h"
#include "Others/Console.h"

namespace Maple
{
	VulkanFrameBuffer::VulkanFrameBuffer(const FrameBufferInfo& info)
		:info(info)
	{
		std::vector<VkImageView> attachments;

		//apply attachment for the framebuffer
		for (uint32_t i = 0; i < info.attachments.size(); i++)
		{
			switch (info.types[i])
			{
				//color attachment
			case TextureType::COLOR: attachments.push_back(static_cast<VulkanTexture2D*>(info.attachments[i].get())->getImageView()); break;
				//depth attachment for depth test					
			case TextureType::DEPTH: attachments.push_back(static_cast<VulkanTextureDepth*>(info.attachments[i].get())->getImageView()); break;
		
			case TextureType::DEPTHARRAY: attachments.push_back(static_cast<VulkanTextureDepthArray*>(info.attachments[i].get())->getImageView(info.layer)); break;

			case TextureType::CUBE: attachments.push_back(static_cast<VulkanTextureCube*>(info.attachments[i].get())->getImageView()); break;			
			}
		}

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = *static_cast<VulkanRenderPass*>(info.renderPass.get());
		framebufferInfo.attachmentCount = attachments.size();
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = info.width;
		framebufferInfo.height = info.height;
		framebufferInfo.layers = 1;
		framebufferInfo.pNext = nullptr;
		framebufferInfo.flags = 0;

		width = info.width;
		height = info.height;

		VK_CHECK_RESULT(vkCreateFramebuffer(*VulkanDevice::get(), &framebufferInfo, nullptr, &buffer));
	}

	VulkanFrameBuffer::~VulkanFrameBuffer()
	{
		vkDestroyFramebuffer(*VulkanDevice::get(), buffer, VK_NULL_HANDLE);
	}

};