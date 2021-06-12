//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 
////////////////////////////////////////////////////////////////////////////// 


#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include "Engine/TextureFormat.h"
#include "Engine/Renderer/RenderParam.h"
namespace Maple
{
	class RenderPass;
	class Texture;

	struct FrameBufferInfo
	{
		uint32_t width;
		uint32_t height;
		uint32_t layer = 0;
		std::vector<std::shared_ptr<Texture>> attachments;
		std::vector<TextureType> types;
		std::shared_ptr<RenderPass> renderPass;
		bool screenFBO = false;
		auto hashCode() const ->size_t;
	};

	class FrameBuffer
	{
	public:
		virtual ~FrameBuffer() = default;
		inline auto getWidth() const { return width; }
		inline auto getHeight() const { return height; }
		static auto create(const FrameBufferInfo& info)->std::shared_ptr<FrameBuffer>;
		static std::unordered_map<size_t, std::shared_ptr<FrameBuffer>> frameBufferCache;
	protected:
		uint32_t width = 0;
		uint32_t height = 0;
	};

};