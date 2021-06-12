//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 
////////////////////////////////////////////////////////////////////////////// 


#include "Engine/Vulkan/VulkanTexture.h"

namespace Maple 
{
	auto TextureCube::create(const std::string& files) -> std::shared_ptr<TextureCube>
	{
		return std::make_shared<VulkanTextureCube>(files);
	}

	auto TextureCube::create(int32_t size) -> std::shared_ptr<TextureCube>
	{
		return std::make_shared<VulkanTextureCube>(size);
	}

	auto TextureCube::update(CommandBuffer* commandBuffer, FrameBuffer* framebuffer, int32_t cubeIndex) -> void
	{

	}

	auto Texture2D::getDefaultTexture() -> std::shared_ptr<Texture2D>
	{
		uint32_t whiteTextureData = 0xffffffff;
		static std::shared_ptr<Texture2D> default2D = Texture2D::createFromSource(1, 1, (uint8_t*)&whiteTextureData);
		return default2D;
	}

	auto Texture2D::create(const std::string& name, const std::string& fileName) ->std::shared_ptr<Texture2D>
	{
		return std::make_shared<VulkanTexture2D>(name, fileName);
	}

	auto Texture2D::create() ->std::shared_ptr<Texture2D>
	{
		return std::make_shared<VulkanTexture2D>();
	}

	auto Texture2D::createFromSource(uint32_t w, uint32_t h, const uint8_t* data) ->std::shared_ptr<Texture2D>
	{
		return std::make_shared<VulkanTexture2D>(w, h, (const void*)data);
	}

	auto Texture2D::loadKTXFile(std::string filename, ktxTexture** target) ->ktxResult
	{
		ktxResult result = KTX_SUCCESS;
		result = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, target);
		return result;
	}

	auto TextureDepth::create(uint32_t width, uint32_t height) ->std::shared_ptr<TextureDepth>
	{
		return std::make_shared<VulkanTextureDepth>(width, height);
	}


	auto TextureDepthArray::create(uint32_t width, uint32_t height, uint32_t count) ->std::shared_ptr<TextureDepthArray>
	{
		return std::make_shared<VulkanTextureDepthArray>(width, height, count);
	}

};
