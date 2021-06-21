//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <stdexcept>
#include "ImageLoader.h"

#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_GIF
#define STBI_NO_PNM

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Others/StringUtils.h"
#include <ktx.h>


namespace Maple
{
	auto ImageLoader::loadAsset(const std::string& name, bool mipmaps) -> std::unique_ptr<Image>
	{
		bool hdr = stbi_is_hdr(name.c_str());
		stbi_set_flip_vertically_on_load(1);
		int32_t width;
		int32_t height;
		int32_t channels;
		TextureFormat format = hdr ? TextureFormat::RGBA32 : TextureFormat::RGBA8 ;
		uint8_t* data = hdr ? 
			(uint8_t*)stbi_loadf(name.c_str(), &width, &height, &channels, STBI_rgb_alpha) :
			stbi_load(name.c_str(), &width, &height, &channels, STBI_rgb_alpha);

		uint32_t imageSize = width * height * 4 * (hdr ? sizeof(float) : sizeof(uint8_t));
		assert(data);
		return std::make_unique<Image>(format, width, height, data, imageSize, channels, mipmaps);
	}

}
