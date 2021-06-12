//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 
////////////////////////////////////////////////////////////////////////////// 

#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "Engine/TextureFormat.h"
namespace Maple
{

	class CommandBuffer;
	class FrameBuffer;

	struct AttachmentInfo
	{
		TextureType textureType;
		TextureFormat format;
		bool srgb = false;
	};

	struct RenderPassInfo
	{
		AttachmentInfo* textureType;
		int32_t attachmentCount;
		bool clear = true;
	};

	enum SubPassContents
	{
		INLINE,
		SECONDARY
	};


	class RenderPass
	{
	public:
		virtual ~RenderPass() = default;
		virtual auto getAttachmentCount() const->int32_t = 0;
		inline auto getAttachmentType(int32_t i) const { return attachmentTypes[i]; }

		virtual auto beginRenderPass(CommandBuffer* commandBuffer, const glm::vec4& clearColor, FrameBuffer* frame, SubPassContents contents, uint32_t width, uint32_t height, bool beginCommandBuffer = false) -> void = 0;
		virtual auto endRenderpass(CommandBuffer* commandBuffer, bool endCommandBuffer = false) -> void = 0;


		static auto create(const RenderPassInfo& info)->std::shared_ptr<RenderPass>;

	protected:
		std::vector<TextureType> attachmentTypes;
	};
};