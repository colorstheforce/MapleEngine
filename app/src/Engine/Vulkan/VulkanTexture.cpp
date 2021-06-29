//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 
////////////////////////////////////////////////////////////////////////////// 

#include "VulkanTexture.h"
#include "VulkanBuffer.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "VulkanFrameBuffer.h"
#include "Others/Console.h"
#include "Others/StringUtils.h"
#include "FileSystem/ImageLoader.h"
#include "FileSystem/Image.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"

#include <ktx.h>
#include <cassert>



namespace Maple
{
	auto generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) -> void
	{
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(*VulkanDevice::get()->getPhysicalDevice(), imageFormat, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
			LOGE("Texture image format does not support linear blitting!");
		}

		VkCommandBuffer commandBuffer = VulkanHelper::beginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0,
				nullptr,
				0,
				nullptr,
				1,
				&barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer,
				image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0,
				nullptr,
				0,
				nullptr,
				1,
				&barrier);

			if (mipWidth > 1)
				mipWidth /= 2;
			if (mipHeight > 1)
				mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		//barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier);

		VulkanHelper::endSingleTimeCommands(commandBuffer);
	}



	VulkanTexture2D::VulkanTexture2D(uint32_t width, uint32_t height, const void* data, TextureParameters parameters, TextureLoadOptions loadOptions)
		:
		parameters(parameters),
		loadOptions(loadOptions),
		data((const uint8_t*)data)

	{
		fileName = "null";


		this->width = width;
		this->height = height;
		deleteImage = load();


		textureImageView = VulkanHelper::createImageView(
			textureImage,
			VkConverter::textureFormatToVK(parameters.format, parameters.srgb)
			, mipLevels, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 1
		);

		auto phyDevice = VulkanDevice::get()->getPhysicalDevice();

		textureSampler = VulkanHelper::createTextureSampler(
			VkConverter::textureFilterToVK(parameters.magFilter),
			VkConverter::textureFilterToVK(parameters.minFilter), 0.0f, static_cast<float>(mipLevels), true,
			phyDevice->getProperties().limits.maxSamplerAnisotropy,
			VkConverter::textureWrapToVK(parameters.wrap),
			VkConverter::textureWrapToVK(parameters.wrap),
			VkConverter::textureWrapToVK(parameters.wrap));

		updateDescriptor();
	}

	VulkanTexture2D::VulkanTexture2D(const std::string& name, const std::string& filename, TextureParameters parameters, TextureLoadOptions loadOptions)
		:parameters(parameters),
		loadOptions(loadOptions)
	{

		this->fileName = filename;

		if (StringUtils::endWith(fileName, "ktx"))
		{
			loadKTX();
			deleteImage = true;
		}
		else 
		{
			deleteImage = load();
			if (!deleteImage)
				return;

			textureImageView = VulkanHelper::createImageView(
				textureImage,
				VkConverter::textureFormatToVK(this->parameters.format, parameters.srgb)
				, mipLevels, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 1
			);

			auto phyDevice = VulkanDevice::get()->getPhysicalDevice();

			textureSampler = VulkanHelper::createTextureSampler(
				VkConverter::textureFilterToVK(parameters.magFilter),
				VkConverter::textureFilterToVK(parameters.minFilter), 0.0f, static_cast<float>(mipLevels), true,
				phyDevice->getProperties().limits.maxSamplerAnisotropy,
				VkConverter::textureWrapToVK(parameters.wrap),
				VkConverter::textureWrapToVK(parameters.wrap),
				VkConverter::textureWrapToVK(parameters.wrap));

			updateDescriptor();
		}
	
	}

	VulkanTexture2D::VulkanTexture2D(VkImage image, VkImageView imageView)
		:textureImage(image), textureImageView(imageView)
	{
		deleteImage = false;
		updateDescriptor();
	}

	VulkanTexture2D::VulkanTexture2D()
	{
		fileName = "null";

		width = 0;
		height = 0;
		mipLevels = 1;
		parameters = TextureParameters();
		loadOptions = TextureLoadOptions();
		deleteImage = false;
		textureSampler = VulkanHelper::createTextureSampler(
			VkConverter::textureFilterToVK(parameters.magFilter),
			VkConverter::textureFilterToVK(parameters.minFilter),
			0.0f, static_cast<float>(mipLevels), true, VulkanDevice::get()->getPhysicalDevice()->getProperties().limits.maxSamplerAnisotropy,
			VkConverter::textureWrapToVK(parameters.wrap),
			VkConverter::textureWrapToVK(parameters.wrap),
			VkConverter::textureWrapToVK(parameters.wrap)
		);

		updateDescriptor();
	}

	VulkanTexture2D::~VulkanTexture2D()
	{
		if (textureSampler)
			vkDestroySampler(*VulkanDevice::get(), textureSampler, nullptr);

		if (textureImageView)
			vkDestroyImageView(*VulkanDevice::get(), textureImageView, nullptr);

		if (deleteImage)
		{
			vkDestroyImage(*VulkanDevice::get(), textureImage, nullptr);
			if (textureImageMemory)
			{
				vkFreeMemory(*VulkanDevice::get(), textureImageMemory, nullptr);
			}
		}
	}

	auto VulkanTexture2D::update(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint8_t* data) -> void
	{
		auto stagingBuffer = std::make_unique<VulkanBuffer>(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, w * h * 4, data);
		VulkanHelper::transitionImageLayout(textureImage, VkConverter::textureFormatToVK(parameters.format, parameters.srgb), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
		VulkanHelper::copyBufferToImage(stagingBuffer->getBuffer(), textureImage, static_cast<uint32_t>(w), static_cast<uint32_t>(h),x,y);
		VulkanHelper::transitionImageLayout(textureImage, VkConverter::textureFormatToVK(parameters.format),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			mipLevels);
	}

	auto VulkanTexture2D::loadKTX() -> void
	{
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		ktxTexture* ktxTexture;
		ktxResult result = loadKTXFile(fileName, &ktxTexture);
		assert(result == KTX_SUCCESS);

		width = ktxTexture->baseWidth;
		height = ktxTexture->baseHeight;
		mipLevels = ktxTexture->numLevels;
		layerCount = ktxTexture->numLayers;

		ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
		ktx_size_t ktxTextureSize = ktxTexture_GetSize(ktxTexture);

		
		VkCommandBuffer cmdBuffer = VulkanHelper::beginSingleTimeCommands();
		auto stagingBuffer = std::make_unique<VulkanBuffer>(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, ktxTextureSize, nullptr);
		stagingBuffer->setData(ktxTextureSize, ktxTextureData);

		// Setup buffer copy regions for each mip level
		std::vector<VkBufferImageCopy> bufferCopyRegions;

		for (uint32_t layer = 0; layer < layerCount; layer++)
		{
			for (uint32_t level = 0; level < mipLevels; level++)
			{

				ktx_size_t offset;
				KTX_error_code result = ktxTexture_GetImageOffset(ktxTexture, level, layer, 0, &offset);
				assert(result == KTX_SUCCESS);

				VkBufferImageCopy bufferCopyRegion = {};
				bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				bufferCopyRegion.imageSubresource.mipLevel = level;
				bufferCopyRegion.imageSubresource.baseArrayLayer = layer;
				bufferCopyRegion.imageSubresource.layerCount = 1;
				bufferCopyRegion.imageExtent.width = ktxTexture->baseWidth >> level;
				bufferCopyRegion.imageExtent.height = ktxTexture->baseHeight >> level;
				bufferCopyRegion.imageExtent.depth = 1;
				bufferCopyRegion.bufferOffset = offset;


				bufferCopyRegions.emplace_back(bufferCopyRegion);
			}
		}
		VulkanHelper::createImage(width, height, mipLevels, format, VK_IMAGE_TYPE_2D
			, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory, layerCount, 0
		);


		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = mipLevels;
		subresourceRange.layerCount = 1;

		// Image barrier for optimal image (target)
		// Optimal image will be used as destination for the copy
		VulkanHelper::setImageLayout(
			cmdBuffer,
			textureImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subresourceRange);

		vkCmdCopyBufferToImage(
			cmdBuffer,
			stagingBuffer->getBuffer(),
			textureImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<uint32_t>(bufferCopyRegions.size()),
			bufferCopyRegions.data());

		// Change texture image layout to shader read after all mip levels have been copied
		this->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VulkanHelper::setImageLayout(
			cmdBuffer,
			textureImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			imageLayout,
			subresourceRange);

		VulkanHelper::endSingleTimeCommands(cmdBuffer);
		ktxTexture_Destroy(ktxTexture);
		textureSampler = VulkanHelper::createTextureSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, 0.0f, static_cast<float>(mipLevels), true, VulkanDevice::get()->getPhysicalDevice()->getProperties().limits.maxSamplerAnisotropy, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);
		textureImageView = VulkanHelper::createImageView(textureImage, format, mipLevels, VK_IMAGE_VIEW_TYPE_CUBE, VK_IMAGE_ASPECT_COLOR_BIT, layerCount);
		updateDescriptor();
	}

	auto VulkanTexture2D::load() -> bool
	{
		auto imageSize = 0;
		const uint8_t* pixel = nullptr;
		std::unique_ptr<Maple::Image> image;
		if (data != nullptr)
		{
			parameters.format = TextureFormat::RGBA8;
			imageSize = width * height * 4;
			pixel = data;
		}
		else
		{
			image = Maple::ImageLoader::loadAsset(fileName);
			width = image->getWidth();
			height = image->getHeight();
			imageSize = image->getImageSize();
			pixel = reinterpret_cast<const uint8_t*>(image->getData());
		}

		mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

		if (!loadOptions.generateMipMaps) 
		{
			mipLevels = 1;
		}


		auto size = VulkanHelper::createImage(width, height, mipLevels, 
			VkConverter::textureFormatToVK(parameters.format, parameters.srgb), 
			VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory, 1, 0);
		
		
		auto stagingBuffer = std::make_unique<VulkanBuffer>(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, imageSize, pixel);
		
		VulkanHelper::transitionImageLayout(textureImage, VkConverter::textureFormatToVK(parameters.format, parameters.srgb), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
		VulkanHelper::copyBufferToImage(stagingBuffer->getBuffer(), textureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
		VulkanHelper::transitionImageLayout(textureImage, VkConverter::textureFormatToVK(parameters.format), 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			 mipLevels);

		if(loadOptions.generateMipMaps)
			generateMipmaps(textureImage, VkConverter::textureFormatToVK(parameters.format, parameters.srgb), width, height, mipLevels);

		return true;
	}

	auto VulkanTexture2D::updateDescriptor() -> void
	{
		descriptor.sampler = textureSampler;
		descriptor.imageView = textureImageView;
		descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;//VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;//VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	auto VulkanTexture2D::buildTexture(TextureFormat internalformat, uint32_t width, uint32_t height, bool srgb, bool depth, bool samplerShadow) -> void
	{
		if (textureSampler)
			vkDestroySampler(*VulkanDevice::get(), textureSampler, nullptr);

		if (textureImageView)
			vkDestroyImageView(*VulkanDevice::get(), textureImageView, nullptr);

		if (deleteImage)
		{
			vkDestroyImage(*VulkanDevice::get(), textureImage, nullptr);

			if (textureImageMemory)
			{
				vkFreeMemory(*VulkanDevice::get(), textureImageMemory, nullptr);
			}
		}

		this->width = width;
		this->height = height;
		handle = 0;
		deleteImage = true;
		mipLevels = 1;

		VulkanHelper::createImage(width, height, mipLevels, VkConverter::textureFormatToVK(internalformat, srgb), VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory, 1, 0);

		textureImageView = VulkanHelper::createImageView(textureImage, VkConverter::textureFormatToVK(internalformat, srgb), mipLevels, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		textureSampler = VulkanHelper::createTextureSampler(
			VkConverter::textureFilterToVK(parameters.magFilter),
			VkConverter::textureFilterToVK(parameters.minFilter),
			0.0f, static_cast<float>(mipLevels), true,
			VulkanDevice::get()->getPhysicalDevice()->getProperties().limits.maxSamplerAnisotropy,
			VkConverter::textureWrapToVK(parameters.wrap),
			VkConverter::textureWrapToVK(parameters.wrap),
			VkConverter::textureWrapToVK(parameters.wrap)
		);

		updateDescriptor();
	}

	VulkanTextureDepth::VulkanTextureDepth(uint32_t width, uint32_t height)
		
	{
		this->width = width;
		this->height = height;
		init();
	}

	auto VulkanTextureDepth::release() -> void
	{
		auto device = VulkanDevice::get();

		if (textureSampler)
			vkDestroySampler(*device, textureSampler, nullptr);

		vkDestroyImageView(*device, textureImageView, nullptr);
		vkDestroyImage(*device, textureImage, nullptr);
		vkFreeMemory(*device, textureImageMemory, nullptr);
	}


	VulkanTextureDepth::~VulkanTextureDepth()
	{
		release();
	}

	auto VulkanTextureDepth::resize(uint32_t width, uint32_t height) -> void
	{
		this->width = width;
		this->height = height;
		release();
		init();
	}

	auto VulkanTextureDepth::updateDescriptor() -> void
	{
		descriptor.sampler = textureSampler;
		descriptor.imageView = textureImageView;
		descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		//must be VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL
			//VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	auto VulkanTextureDepth::init() -> void
	{
		VkFormat depthFormat = VkConverter::findDepthFormat();
		VulkanHelper::createImage(width, height, 1, depthFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory, 1, 0);
		textureImageView = VulkanHelper::createImageView(textureImage, depthFormat, 1, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

		VulkanHelper::transitionImageLayout(textureImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		textureSampler = VulkanHelper::createTextureSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, 0.0f, 1.0f, true,
			VulkanDevice::get()->getPhysicalDevice()->getProperties().limits.maxSamplerAnisotropy,
			VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

		updateDescriptor();
	}



	VulkanTextureCube::VulkanTextureCube(const std::string& filePath)
		: size(0)
	{
		files[0] = filePath;
		load(1);
		updateDescriptor();
	}



	VulkanTextureCube::VulkanTextureCube(int32_t size, TextureFormat format,int32_t numMips)
	{
		this->numMips = numMips;
		width = size;
		height = size;
		parameters.format = format;
		init();
		deleteImg = false;
	}

	VulkanTextureCube::~VulkanTextureCube()
	{
		if (textureSampler)
			vkDestroySampler(*VulkanDevice::get(), textureSampler, nullptr);

		if (textureImageView)
			vkDestroyImageView(*VulkanDevice::get(), textureImageView, nullptr);

		if (deleteImg)
		{
			vkDestroyImage(*VulkanDevice::get(), textureImage, nullptr);
			vkFreeMemory(*VulkanDevice::get(), textureImageMemory, nullptr);
		}
	}

	auto VulkanTextureCube::updateDescriptor() -> void
	{
		descriptor.sampler = textureSampler;
		descriptor.imageView = textureImageView;
		descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	auto VulkanTextureCube::load(uint32_t mips) -> void
	{
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		ktxResult result;
		ktxTexture* ktxTexture;
		result = ktxTexture_CreateFromNamedFile(files[0].c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
		assert(result == KTX_SUCCESS);
		width = ktxTexture->baseWidth;
		height = ktxTexture->baseHeight;
		numMips = ktxTexture->numLevels;

		ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
		ktx_size_t ktxTextureSize = ktxTexture_GetSize(ktxTexture);

		auto stagingBuffer = std::make_unique<VulkanBuffer>(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, ktxTextureSize,nullptr);

		stagingBuffer->setData(ktxTextureSize, ktxTextureData);

		VulkanHelper::createImage(width, height, numMips, format, VK_IMAGE_TYPE_2D
			, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory, 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
		);


		VkCommandBuffer cmdBuffer = VulkanHelper::beginSingleTimeCommands();

		std::vector<VkBufferImageCopy> bufferCopyRegions;
		uint32_t offset = 0;

		for (uint32_t face = 0; face < 6; face++)
		{
			for (uint32_t level = 0; level < numMips; level++)
			{
				ktx_size_t offset;
				KTX_error_code ret = ktxTexture_GetImageOffset(ktxTexture, level, 0, face, &offset);
				assert(ret == KTX_SUCCESS);
				VkBufferImageCopy bufferCopyRegion = {};
				bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				bufferCopyRegion.imageSubresource.mipLevel = level;
				bufferCopyRegion.imageSubresource.baseArrayLayer = face;
				bufferCopyRegion.imageSubresource.layerCount = 1;
				bufferCopyRegion.imageExtent.width =  ktxTexture->baseWidth >> level;
				bufferCopyRegion.imageExtent.height = ktxTexture->baseHeight >> level;
				bufferCopyRegion.imageExtent.depth = 1;
				bufferCopyRegion.bufferOffset = offset;
				bufferCopyRegions.emplace_back(bufferCopyRegion);
			}
		}

		// Image barrier for optimal image (target)
		// Set initial layout for all array layers (faces) of the optimal (target) tiled texture
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = numMips;
		subresourceRange.layerCount = 6;

		VulkanHelper::setImageLayout(
			cmdBuffer,
			textureImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subresourceRange);

		// Copy the cube map faces from the staging buffer to the optimal tiled image
		vkCmdCopyBufferToImage(
			cmdBuffer,
			stagingBuffer->getBuffer(),
			textureImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<uint32_t>(bufferCopyRegions.size()),
			bufferCopyRegions.data());

		// Change texture image layout to shader read after all faces have been copied
		imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VulkanHelper::setImageLayout(
			cmdBuffer,
			textureImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			imageLayout,
			subresourceRange);

		VulkanHelper::endSingleTimeCommands(cmdBuffer);

		textureSampler =	VulkanHelper::createTextureSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, 0.0f, static_cast<float>(numMips), true, VulkanDevice::get()->getPhysicalDevice()->getProperties().limits.maxSamplerAnisotropy, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);
		textureImageView =	VulkanHelper::createImageView(textureImage, format, numMips, VK_IMAGE_VIEW_TYPE_CUBE, VK_IMAGE_ASPECT_COLOR_BIT, 6);
		deleteImg = true;
		ktxTexture_Destroy(ktxTexture);
	}

	auto VulkanTextureCube::update(CommandBuffer* commandBuffer, FrameBuffer* framebuffer, int32_t cubeIndex, int32_t mipmapLevel) -> void
	{
		VkCommandBuffer cmd = *static_cast<VulkanCommandBuffer*>(commandBuffer);
		VulkanFrameBuffer* frameBuffer = static_cast<VulkanFrameBuffer*>(framebuffer);

		VkImage colorImage = nullptr;
		auto& info = frameBuffer->getFrameBufferInfo();
		switch (frameBuffer->getFrameBufferInfo().types[0])
		{
		case TextureType::COLOR:
			colorImage = static_cast<VulkanTexture2D*>(info.attachments[0].get())->getImage();
			break;

		case TextureType::DEPTH:
			colorImage = static_cast<VulkanTextureDepth*>(info.attachments[0].get())->getImage();
			break;

		case TextureType::DEPTHARRAY:
			colorImage = static_cast<VulkanTextureDepthArray*>(info.attachments[0].get())->getImage();
			break;

		case TextureType::CUBE:
			colorImage = static_cast<VulkanTextureCube*>(info.attachments[0].get())->getImage();
			break;
		}

		assert(colorImage != nullptr);

		VulkanHelper::setImageLayout(
			cmd,
			colorImage,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		VkImageSubresourceRange cubeFaceSubresourceRange = {};
		cubeFaceSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		cubeFaceSubresourceRange.baseMipLevel = 0;
		cubeFaceSubresourceRange.baseArrayLayer = cubeIndex;
		cubeFaceSubresourceRange.layerCount = 6;
		cubeFaceSubresourceRange.levelCount = numMips;

		// Change image layout of one cubemap face to transfer destination
		VulkanHelper::setImageLayout(
			cmd,
			textureImage,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
			/*VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,*/
		);

		// Copy region for transfer from framebuffer to cube face
		VkImageCopy copyRegion = {};

		copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.srcSubresource.baseArrayLayer = 0;
		copyRegion.srcSubresource.mipLevel = 0;
		copyRegion.srcSubresource.layerCount = 1;
		copyRegion.srcOffset = { 0, 0, 0 };

		copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.dstSubresource.baseArrayLayer = cubeIndex;
		copyRegion.dstSubresource.mipLevel = mipmapLevel;
		copyRegion.dstSubresource.layerCount = 1;
		copyRegion.dstOffset = { 0, 0, 0 };

		copyRegion.extent.width = width;
		copyRegion.extent.height = height;
		copyRegion.extent.depth = 1;

		// Put image copy into command buffer
		vkCmdCopyImage(
			cmd,
			colorImage,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			textureImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&copyRegion);

		// Transform framebuffer color attachment back
		VulkanHelper::setImageLayout(
			cmd,
			colorImage,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		// Change image layout of copied face to shader read
		VulkanHelper::setImageLayout(
			cmd,
			textureImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			cubeFaceSubresourceRange);

	}

	auto VulkanTextureCube::init() -> void
	{
		VkFormat format = VkConverter::textureFormatToVK(parameters.format);

		VulkanHelper::createImage(
			width, height, numMips, format, 
			VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory,
			6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
		);

		VkCommandBuffer cmdBuffer = VulkanHelper::beginSingleTimeCommands();

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = numMips;
		subresourceRange.layerCount = 6;

		VulkanHelper::setImageLayout(
			cmdBuffer,
			textureImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			subresourceRange);

		VulkanHelper::endSingleTimeCommands(cmdBuffer);

	
		textureSampler = VulkanHelper::createTextureSampler(
			VK_FILTER_LINEAR, 
			VK_FILTER_LINEAR, 
			0.0f, static_cast<float>(numMips), 
			true, 
			VulkanDevice::get()->getPhysicalDevice()->getProperties().limits.maxSamplerAnisotropy, 
			VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	
		textureImageView = VulkanHelper::createImageView(textureImage, format, numMips, 
			VK_IMAGE_VIEW_TYPE_CUBE, VK_IMAGE_ASPECT_COLOR_BIT, 6);

		updateDescriptor();
	}


	VulkanTextureDepthArray::VulkanTextureDepthArray(uint32_t width, uint32_t height, uint32_t count)
		:width(width),height(height),count(count)
	{
		init();
	}

	VulkanTextureDepthArray::~VulkanTextureDepthArray()
	{
		vkDestroyImageView(*VulkanDevice::get(), textureImageView, nullptr);
		vkDestroyImage(*VulkanDevice::get(), textureImage, nullptr);
		vkFreeMemory  (*VulkanDevice::get(), textureImageMemory, nullptr);
		vkDestroySampler(*VulkanDevice::get(), textureSampler, nullptr);
		for (uint32_t i = 0; i < count; i++)
		{
			vkDestroyImageView(*VulkanDevice::get(), imageViews[i], nullptr);
		}
	}

	auto VulkanTextureDepthArray::resize(uint32_t width, uint32_t height, uint32_t count) -> void 
	{
		this->width = width;
		this->height = height;
		this->count = count;

		if (textureSampler)
			vkDestroySampler(*VulkanDevice::get(), textureSampler, nullptr);

		vkDestroyImageView(*VulkanDevice::get(), textureImageView, nullptr);
		vkDestroyImage(*VulkanDevice::get(), textureImage, nullptr);
		vkFreeMemory(*VulkanDevice::get(), textureImageMemory, nullptr);

		init();
	}

	auto VulkanTextureDepthArray::getHandleArray(uint32_t index) -> void* 
	{
		descriptor.imageView = getImageView(index);
		return (void*)&descriptor;
	}

	auto VulkanTextureDepthArray::updateDescriptor() -> void
	{
		descriptor.sampler = textureSampler;
		descriptor.imageView = textureImageView;
		descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	auto VulkanTextureDepthArray::init() -> void 
	{
		auto depthFormat = VkConverter::findDepthFormat();
		VulkanHelper::createImage(width, height, 1, depthFormat, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,textureImage,textureImageMemory, count, 0);
		textureImageView = VulkanHelper::createImageView(textureImage, depthFormat, 1, VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_ASPECT_DEPTH_BIT, count);
		for (uint32_t i = 0; i < count; i++)
		{
			imageViews.emplace_back(VulkanHelper::createImageView(textureImage, depthFormat, 1, VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_ASPECT_DEPTH_BIT, 1, i));
		}
		VulkanHelper::transitionImageLayout(textureImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		textureSampler = VulkanHelper::createTextureSampler();
		updateDescriptor();
	}

};