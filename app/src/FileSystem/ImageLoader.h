//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <memory>
#include <string>
#include "Image.h"

namespace Maple
{
    class ImageLoader final 
    {
    public:
        static auto loadAsset(const std::string& name, bool mipmaps = true) -> std::unique_ptr<Image>;
    };
}

