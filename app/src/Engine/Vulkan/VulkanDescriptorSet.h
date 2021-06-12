//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VulkanHelper.h"
#include "Engine/Renderer/RenderParam.h"
#include "Engine/Interface/DescriptorSet.h"

namespace Maple
{

	class VulkanShader;

	class VulkanDescriptorSet final : public DescriptorSet
	{
	public:
		VulkanDescriptorSet(const DescriptorInfo& info);
		virtual ~VulkanDescriptorSet();

		auto update(const std::vector<ImageInfo>& imageInfos, const std::vector<BufferInfo>& bufferInfos) -> void  override;
		auto update(const std::vector<ImageInfo>& imageInfos) -> void  override;
		auto update(const std::vector<BufferInfo>& bufferInfos) -> void  override;


		virtual auto setDynamicOffset(uint32_t offset) -> void;
		virtual auto getDynamicOffset() const->uint32_t;

		inline auto isDynamic()const { return dynamic; }

		/*inline auto& getPushConstants() const { return pushConstants; }*/
		inline auto getDescriptorSet() { return &descriptorSet; }

		autoUnpack(descriptorSet);
	private:
		auto updateInternal(const std::vector<ImageInfo>* imageInfos, const std::vector<BufferInfo>* bufferInfos) -> void;

		VkDescriptorSet descriptorSet;
		uint32_t dynamicOffset = 0;
		std::shared_ptr<VulkanShader> shader;
		bool dynamic = false;
		VkDescriptorBufferInfo* bufferInfoPool = nullptr;
		VkDescriptorImageInfo* imageInfoPool = nullptr;
		VkWriteDescriptorSet* writeDescriptorSetPool = nullptr;
	};
};