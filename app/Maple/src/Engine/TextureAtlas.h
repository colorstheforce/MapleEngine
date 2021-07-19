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
		TextureAtlas(uint32_t w = 4096, uint32_t h = 4096);

		auto addSprite(const std::string& file)->Quad2D*;
		auto addSprite(const std::string& uniqueName,const std::vector<uint8_t> &, uint32_t w,uint32_t h)->Quad2D*;

		inline auto getTexture() { return texture; }
		inline auto getUsage() { return usage; }

	private:
		
		auto update(const std::string& uniqueName,const uint8_t * buffer,uint32_t w,uint32_t h)->Quad2D*;

		size_t rlid = 1;
		size_t wasted = 0;
		float usage = 0.f;
		std::shared_ptr<Texture2D> texture;
		std::unordered_map<std::string, Quad2D> offsets;
		QuadTree<size_t, LeftOver> leftovers;

		static const int16_t MINLOSIZE = 4;

		std::pair<int16_t, int16_t> border;
		std::pair<int16_t, int16_t> yRange;


	};
};