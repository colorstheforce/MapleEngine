
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "VulkanDescriptorSet.h"
#include "VulkanHelper.h"
#include "VulkanDevice.h"
#include "VulkanPipeline.h"
#include "Others/Console.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "VulkanUniformBuffer.h"
#include <array>

#define MAX_BUFFER_INFOS 16
#define MAX_IMAGE_INFOS 16
#define MAX_WRITE_DESCTIPTORS 16

namespace Maple
{

	/**
	 * create the descriptor set
	 */
	VulkanDescriptorSet::VulkanDescriptorSet(const DescriptorInfo& info)
	{

		auto vkPipeline = static_cast<VulkanPipeline*>(info.pipeline);

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = vkPipeline->getDescriptorPool();
		/**
		 * the pipeline would contain different layout.
		 */
		descriptorSetAllocateInfo.pSetLayouts = vkPipeline->getDescriptorLayout(info.layoutIndex);
		descriptorSetAllocateInfo.descriptorSetCount = info.count;
		descriptorSetAllocateInfo.pNext = nullptr;

		VK_CHECK_RESULT(vkAllocateDescriptorSets(*VulkanDevice::get(), &descriptorSetAllocateInfo, &descriptorSet));

		bufferInfoPool = new VkDescriptorBufferInfo[MAX_BUFFER_INFOS];
		imageInfoPool = new VkDescriptorImageInfo[MAX_IMAGE_INFOS];
		writeDescriptorSetPool = new VkWriteDescriptorSet[MAX_WRITE_DESCTIPTORS];
	}

	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
		delete[] bufferInfoPool;
		delete[] imageInfoPool;
		delete[] writeDescriptorSetPool;
	}

	auto VulkanDescriptorSet::update(const std::vector<ImageInfo>& imageInfos, const std::vector<BufferInfo>& bufferInfos) -> void
	{
		updateInternal(&imageInfos, &bufferInfos);
	}

	auto VulkanDescriptorSet::update(const std::vector<ImageInfo>& imageInfos) -> void
	{
		updateInternal(&imageInfos, nullptr);
	}
	auto VulkanDescriptorSet::update(const std::vector<BufferInfo>& bufferInfo) -> void
	{
		updateInternal(nullptr, &bufferInfo);
	}


	auto VulkanDescriptorSet::setDynamicOffset(uint32_t offset) -> void
	{
		this->dynamicOffset = offset;
	}

	auto VulkanDescriptorSet::getDynamicOffset() const -> uint32_t
	{
		return dynamicOffset;
	}

	auto VulkanDescriptorSet::updateInternal(const std::vector<ImageInfo>* imageInfos, const std::vector<BufferInfo>* bufferInfos) -> void
	{
		dynamic = false;
		int32_t descriptorWritesCount = 0;

		/**
		 * update the images (texture sampler)
		 */
		if (imageInfos != nullptr)
		{
			int32_t imageIndex = 0;

			for (auto& imageInfo : *imageInfos)
			{
				int32_t descriptorCount = 0;
				for (auto i = 0; i < imageInfo.textures.size(); i++)
				{
					if (imageInfo.textures[i] != nullptr) {
						VkDescriptorImageInfo& des = *static_cast<VkDescriptorImageInfo*>(imageInfo.textures[i]->getHandle());
						imageInfoPool[i + imageIndex] = des;
						descriptorCount++;
					}
				}


				VkWriteDescriptorSet writeDescriptorSet{};
				writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSet.dstSet = descriptorSet;
				writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeDescriptorSet.dstBinding = imageInfo.binding;
				writeDescriptorSet.pImageInfo = &imageInfoPool[imageIndex];
				writeDescriptorSet.descriptorCount = descriptorCount;

				writeDescriptorSetPool[descriptorWritesCount] = writeDescriptorSet;
				imageIndex++;
				descriptorWritesCount++;
			}
		}
		/**
		 *  update others uniform data
		 */
		if (bufferInfos != nullptr)
		{
			int index = 0;

			for (auto& bufferInfo : *bufferInfos)
			{
				bufferInfoPool[index].buffer = std::static_pointer_cast<VulkanUniformBuffer>(bufferInfo.buffer)->getBuffer();
				bufferInfoPool[index].offset = bufferInfo.offset;
				bufferInfoPool[index].range = bufferInfo.size;

				VkWriteDescriptorSet writeDescriptorSet{};
				writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSet.dstSet = descriptorSet;
				writeDescriptorSet.descriptorType = VkConverter::descriptorTypeToVK(bufferInfo.type);
				writeDescriptorSet.dstBinding = bufferInfo.binding;
				writeDescriptorSet.pBufferInfo = &bufferInfoPool[index];
				writeDescriptorSet.descriptorCount = 1;

				writeDescriptorSetPool[descriptorWritesCount] = writeDescriptorSet;
				index++;
				descriptorWritesCount++;

				if (bufferInfo.type == DescriptorType::UNIFORM_BUFFER_DYNAMIC)
					dynamic = true;
			}
		}

		vkUpdateDescriptorSets(*VulkanDevice::get(), descriptorWritesCount,
			writeDescriptorSetPool, 0, nullptr);
	}
};