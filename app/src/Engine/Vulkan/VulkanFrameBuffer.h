//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 

////////////////////////////////////////////////////////////////////////////// 


#pragma once

#include <memory>
#include "Engine/Interface/FrameBuffer.h"
#include "VulkanHelper.h"
namespace Maple
{
	
	class VulkanFrameBuffer : public FrameBuffer
	{
	public:
		VulkanFrameBuffer(const FrameBufferInfo& info);
		~VulkanFrameBuffer();
		autoUnpack(buffer);

		inline auto& getFrameBufferInfo() const { return info; }

	private:
		FrameBufferInfo info;
		VkFramebuffer buffer = nullptr;
	};


};