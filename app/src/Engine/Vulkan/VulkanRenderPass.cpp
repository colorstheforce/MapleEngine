
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "VulkanRenderPass.h"
#include "VulkanHelper.h"
#include "VulkanDevice.h"
#include "Others/Console.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFrameBuffer.h"

namespace Maple
{
	VkSubpassContents subPassContentsToVK(SubPassContents contents)
	{
		switch (contents)
		{
		case INLINE:
			return VK_SUBPASS_CONTENTS_INLINE;
		case SECONDARY:
			return VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;
		default:
			return VK_SUBPASS_CONTENTS_INLINE;
		}
	}

	VkAttachmentDescription getAttachmentDescription(AttachmentInfo info, bool clear = true)
	{
		VkAttachmentDescription attachment = {};
		if (info.textureType == TextureType::COLOR)
		{
			attachment.format = VkConverter::textureFormatToVK(info.format, info.srgb);
			attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;//VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;//VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		else if (info.textureType == TextureType::DEPTH)
		{
			attachment.format = VkConverter::findDepthFormat();
			attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
								   //VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else if (info.textureType == TextureType::DEPTHARRAY)
		{
			attachment.format = VkConverter::findDepthFormat();
			attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
				//VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else if (info.textureType == TextureType::CUBE) 
		{
			attachment.format = VK_FORMAT_R32_SFLOAT;
			attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		else
		{
			LOGC("[VULKAN] - Unsupported TextureType - {0}", static_cast<int>(info.textureType));
			return attachment;
		}

		if (clear)
		{
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
		else
		{
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			attachment.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		if (info.textureType == TextureType::CUBE)
		{
			attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.flags = 0;

		return attachment;

	}


	VulkanRenderPass::VulkanRenderPass(const RenderPassInfo& info)
	{
		init(info);
	}

	auto VulkanRenderPass::init(const RenderPassInfo& info) -> void
	{

		std::vector<VkAttachmentDescription> attachments;
		std::vector<VkAttachmentReference> colourAttachmentReferences;
		std::vector<VkAttachmentReference> depthAttachmentReferences;
		clearDepth = false;
		depthOnly = true;
		for (int i = 0; i < info.attachmentCount; i++)
		{
			attachments.emplace_back(getAttachmentDescription(info.textureType[i], info.clear));
			//color 
			attachmentTypes.emplace_back(info.textureType[i].textureType);

			if (info.textureType[i].textureType == TextureType::COLOR)
			{
				VkAttachmentReference colourAttachmentRef = {};
				colourAttachmentRef.attachment = uint32_t(i);
				colourAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colourAttachmentReferences.emplace_back(colourAttachmentRef);
				depthOnly = false;
			}
			else if (info.textureType[i].textureType == TextureType::CUBE) 
			{
				VkAttachmentReference colourAttachmentRef = {};
				colourAttachmentRef.attachment = uint32_t(i);
				colourAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colourAttachmentReferences.emplace_back(colourAttachmentRef);
				depthOnly = false;
			}
			//used for depth test
			else if (info.textureType[i].textureType == TextureType::DEPTH)
			{
				VkAttachmentReference depthAttachmentRef = {};
				depthAttachmentRef.attachment = uint32_t(i);
				depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				depthAttachmentReferences.emplace_back(depthAttachmentRef);
				clearDepth = info.clear;
			}
			else if (info.textureType[i].textureType == TextureType::DEPTHARRAY)
			{
				VkAttachmentReference depthAttachmentRef = {};
				depthAttachmentRef.attachment = uint32_t(i);
				depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				depthAttachmentReferences.emplace_back(depthAttachmentRef);
				clearDepth = info.clear;
			}
		}

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = colourAttachmentReferences.size();
		subpass.pColorAttachments = colourAttachmentReferences.data();
		subpass.pDepthStencilAttachment = depthAttachmentReferences.data();

/*
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
*/


		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


		VkRenderPassCreateInfo renderPassInfo = VulkanHelper::renderPassCreateInfo();
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;


		if (vkCreateRenderPass(*VulkanDevice::get(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
		clearValue = new VkClearValue[info.attachmentCount];
		clearCount = info.attachmentCount;
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		vkDestroyRenderPass(*VulkanDevice::get(), renderPass, nullptr);
		delete[] clearValue;
	}

	auto VulkanRenderPass::beginRenderPass(CommandBuffer* commandBuffer, const glm::vec4 & clearColor, FrameBuffer* frame, SubPassContents contents, uint32_t width, uint32_t height, bool beginCommandBuffer /*= true*/) -> void
	{
		if (beginCommandBuffer)
			commandBuffer->beginRecording();
		commandBuffer->updateViewport(width, height);

		//clear color
		if (!depthOnly)
		{
			for (int32_t i = 0; i < clearCount; i++)
			{
				clearValue[i].color.float32[0] = clearColor.x;
				clearValue[i].color.float32[1] = clearColor.y;
				clearValue[i].color.float32[2] = clearColor.z;
				clearValue[i].color.float32[3] = clearColor.w;
			}
		}

		if (clearDepth)
		{
			clearValue[clearCount - 1].depthStencil = VkClearDepthStencilValue{ 1.0f, 0};
		}

		VkRenderPassBeginInfo rpBegin{};
		rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpBegin.pNext = NULL;
		rpBegin.renderPass = renderPass;
		rpBegin.framebuffer = *static_cast<VulkanFrameBuffer*>(frame);
		rpBegin.renderArea.offset.x = 0;
		rpBegin.renderArea.offset.y = 0;
		rpBegin.renderArea.extent.width = width;
		rpBegin.renderArea.extent.height = height;
		rpBegin.clearValueCount = uint32_t(clearCount);
		rpBegin.pClearValues = clearValue;

		vkCmdBeginRenderPass(*static_cast<VulkanCommandBuffer*>(commandBuffer), &rpBegin, subPassContentsToVK(contents));
	}

	auto VulkanRenderPass::endRenderpass(CommandBuffer* commandBuffer, bool endCommandBuffer /*= true*/) -> void
	{
		vkCmdEndRenderPass(*static_cast<VulkanCommandBuffer*>(commandBuffer));
		if (endCommandBuffer)
			commandBuffer->endRecording();
	}

};