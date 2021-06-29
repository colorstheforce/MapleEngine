//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <memory>
#include <string>
#include "Image.h"
#include "Resources/Resources.h"

namespace Maple
{
    class ImageCache : public Resources<Image> 
    {
        
    };

    class ImageLoader final 
    {
    public:
		static auto loadAsset(const std::string& name, bool mipmaps = true)->std::unique_ptr<Image>;
        static auto loadAsset(const std::string& name, Image * image)-> void;
    };
}

