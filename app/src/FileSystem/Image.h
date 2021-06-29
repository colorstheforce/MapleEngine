
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 

////////////////////////////////////////////////////////////////////////////// 
#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "Engine/TextureFormat.h"


namespace Maple
{
    class Image
    {
    public:
        Image() = default;

        Image(const std::string& fileName);

        Image(TextureFormat initPixelFormat,
            uint32_t width, uint32_t height,
            void* initData, uint32_t imageSize, uint32_t channel, bool mipmaps = true) :
            pixelFormat(initPixelFormat), width(width), height(height), data(initData), size(imageSize), channel(channel), mipmaps(mipmaps)
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
        inline auto isGenerateMipmaps() const  noexcept { return mipmaps; }

        inline auto setPixelFormat(TextureFormat pixelFormat) { this->pixelFormat = pixelFormat; }
        inline auto setWidth(uint32_t width) { this->width = width; }
        inline auto setHeight(uint32_t height) { this->height = height; }
        inline auto setData(void* data) { this->data = data; }
        inline auto setSize(uint32_t size) { this->size = size; }
        inline auto setChannel(uint32_t channel) { this->channel = channel; }
        inline auto setMipmaps(bool mipmaps) { this->mipmaps = mipmaps; }
        inline auto setFileName(std::string fileName) { this->fileName = fileName; }

    protected:
        TextureFormat pixelFormat = TextureFormat::RGBA8;
        uint32_t width = 0;
        uint32_t height = 0;
        void* data = nullptr;
        uint32_t size = 0;
        uint32_t channel = 0;
        bool mipmaps = false;
        std::string fileName;
    };
}
