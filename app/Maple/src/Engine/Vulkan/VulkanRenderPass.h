//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VulkanHelper.h"
#include "Engine/TextureFormat.h"
#include "Engine/Interface/RenderPass.h"

namespace Maple
{

	class VulkanRenderPass : public RenderPass
	{
	public:
		VulkanRenderPass(const RenderPassInfo& info);
		auto init(const RenderPassInfo& info) -> void;
		virtual ~VulkanRenderPass();

		auto beginRenderPass(CommandBuffer* commandBuffer, const glm::vec4 & clearColor, 
			FrameBuffer* frame, SubPassContents contents, uint32_t width, 
			uint32_t height, bool beginCommandBuffer = false) -> void override;
		auto endRenderpass(CommandBuffer* commandBuffer, bool endCommandBuffer = false) -> void override;

		auto getAttachmentCount() const -> int32_t override { return clearCount; }

		autoUnpack(renderPass);
	private:
		
		VkClearValue* clearValue = nullptr;
		int32_t clearCount = 0;
		bool clearDepth = false;
		bool depthOnly = true;
		VkRenderPass renderPass = nullptr;
	};
};