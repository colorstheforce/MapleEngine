
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 

////////////////////////////////////////////////////////////////////////////// 
#pragma once
#include <cstdint>
#include <vector>
#include "Engine/TextureFormat.h"
namespace Maple
{
     class Image 
    {
    public:
        Image() = default;

        Image(TextureFormat initPixelFormat,
              uint32_t width,uint32_t height,
              uint8_t * initData,uint32_t imageSize,uint32_t channel):
            pixelFormat(initPixelFormat), width(width), height(height), data(initData),size(imageSize), channel(channel)
        {
        }

        ~Image() {
            free(data);
        }

        inline auto getPixelFormat() const noexcept { return pixelFormat; }
		inline auto& getWidth() const noexcept { return width; }
		inline auto& getHeight() const noexcept { return height; }
        inline auto& getData() const noexcept { return data; }
        inline auto getImageSize() const noexcept { return size; }
        inline auto getChannel() const  noexcept { return channel; }
     protected:
        TextureFormat pixelFormat = TextureFormat::RGBA8;
        uint32_t width = 0;
        uint32_t height = 0;
        uint8_t* data = nullptr;
        uint32_t size = 0;
        uint32_t channel = 0;
    };





}
