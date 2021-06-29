//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine									//
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////


#pragma once
#include "QuadTree.h"
#include "Quad2D.h"


namespace Maple
{
	class Texture;
	class TextureAtlas
	{
	public:
		TextureAtlas(uint32_t w, uint32_t h);

		auto addSprite(const std::string& file)->Quad2D *;

		inline auto getTexture() { return texture; }

	private:
		
		size_t rlid = 1;
		size_t wasted = 0;
		std::shared_ptr<Texture2D> texture;
		std::unordered_map<std::string, Quad2D> offsets;
		QuadTree<size_t, LeftOver> leftovers;

		static const int16_t MINLOSIZE = 4;

		std::pair<int16_t, int16_t> border;
		std::pair<int16_t, int16_t> yRange;
	};
};