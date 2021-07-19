//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine									//
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////


#pragma once
#include "TextureAtlas.h"
#include <unordered_map>

namespace Maple
{
	class TexturePool final
	{
	public:
		TexturePool() = default;

		auto addSprite(const std::string& file)->Quad2D*;
		auto addSprite(const std::string& uniqueName, const std::vector<uint8_t>&, uint32_t w, uint32_t h)->Quad2D*;

	private:
		auto createTextureAtlas()->TextureAtlas*;

		std::vector<TextureAtlas> atlas;
		std::unordered_map<std::string, TextureAtlas*> mapping;
	};
};