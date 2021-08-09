//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 
////////////////////////////////////////////////////////////////////////////// 

#include "Image.h"
#include "ImageLoader.h"

namespace Maple
{
	Image::Image(const std::string& fileName)
		:fileName(fileName)
	{
		ImageLoader::loadAsset(fileName,this);
	}

};
