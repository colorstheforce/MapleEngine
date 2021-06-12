//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanCommandPool.h"
#include "VulkanFrameBuffer.h"
#include "VulkanRenderPass.h"
#include "Others/Console.h"

namespace Maple
{
	VulkanCommandBuffer::VulkanCommandBuffer(bool primary)
	{
		init(primary);
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		unload();
	}

	/**
	 * init the command buffer
	 */
	auto VulkanCommandBuffer::init(bool primary) -> bool
	{
		
		return init(primary, *VulkanDevice::get()->getCommandPool());
/*

		//create the fence.
		VkFenceCreateInfo fenceCI{};
		fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		VK_CHECK_RESULT(vkCreateFence(*VulkanDevice::get(), &fenceCI, nullptr, &fence));*/

	//	return true;
	}

	auto VulkanCommandBuffer::init(bool primary, VkCommandPool cmdPool) -> bool
	{
		this->primary = primary;
		VkCommandBufferAllocateInfo cmdBufferCI{};

		cmdBufferCI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferCI.commandPool = cmdPool;
		cmdBufferCI.commandBufferCount = 1;
		cmdBufferCI.level = primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;

		VK_CHECK_RESULT(vkAllocateCommandBuffers(*VulkanDevice::get(), &cmdBufferCI, &commandBuffer));
		return true;
	}

	/**
	 * destory the cmd
	 */
	auto VulkanCommandBuffer::unload() -> void
	{
	//	vkDestroyFence(*VulkanDevice::get(), fence, nullptr);
		vkFreeCommandBuffers(*VulkanDevice::get(), *VulkanDevice::get()->getCommandPool(), 1, &commandBuffer);
	}

	auto VulkanCommandBuffer::beginRecording() -> void
	{
		VkCommandBufferBeginInfo beginCI{};
		beginCI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginCI.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginCI));
	}

	auto VulkanCommandBuffer::beginRecordingSecondary(VulkanRenderPass* renderPass, VulkanFrameBuffer* framebuffer) -> void
	{

		VkCommandBufferInheritanceInfo inheritanceInfo{};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.subpass = 0;
		inheritanceInfo.renderPass = *renderPass;
		inheritanceInfo.framebuffer = *framebuffer;

		VkCommandBufferBeginInfo beginCI{};
		beginCI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginCI.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		beginCI.pInheritanceInfo = &inheritanceInfo;

		VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginCI));
	}

	auto VulkanCommandBuffer::endRecording() -> void
	{
		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
	}

	auto VulkanCommandBuffer::execute(bool waitFence) -> void
	{
		executeInternal(VkPipelineStageFlags(), VK_NULL_HANDLE, VK_NULL_HANDLE, waitFence);
	}

	auto VulkanCommandBuffer::executeSecondary(VulkanCommandBuffer* primaryCmdBuffer) -> void
	{
		vkCmdExecuteCommands(*primaryCmdBuffer, 1, &commandBuffer);
	}

	auto VulkanCommandBuffer::updateViewport(uint32_t width, uint32_t height) -> void
	{
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.f;
		viewport.width = static_cast<float>(width);
		viewport.height = static_cast<float>(height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;


		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = { width, height };

		//viewport param
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		//scissor test.
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	auto VulkanCommandBuffer::executeInternal(VkPipelineStageFlags flags, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, bool waitFence) -> void
	{
		uint32_t waitSemaphoreCount = waitSemaphore ? 1 : 0, signalSemaphoreCount = signalSemaphore ? 1 : 0;

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = VK_NULL_HANDLE;
		submitInfo.waitSemaphoreCount = waitSemaphoreCount;
		submitInfo.pWaitSemaphores = &waitSemaphore;
		submitInfo.pWaitDstStageMask = &flags;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.signalSemaphoreCount = signalSemaphoreCount;
		submitInfo.pSignalSemaphores = &signalSemaphore;


		VK_CHECK_RESULT(vkQueueSubmit(VulkanDevice::get()->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));

		/*if (waitFence)
		{
			VK_CHECK_RESULT(vkQueueSubmit(VulkanDevice::get()->getGraphicsQueue(), 1, &submitInfo, fence));
			VK_CHECK_RESULT(vkWaitForFences(*VulkanDevice::get(), 1, &fence, VK_TRUE, UINT64_MAX));
			VK_CHECK_RESULT(vkResetFences(*VulkanDevice::get(), 1, &fence));
		}
		else
		{
			VK_CHECK_RESULT(vkQueueSubmit(VulkanDevice::get()->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
			VK_CHECK_RESULT(vkQueueWaitIdle(VulkanDevice::get()->getGraphicsQueue()));
		}*/
	}
};