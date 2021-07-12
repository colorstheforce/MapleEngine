//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine									//
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////


#include "TextureAtlas.h"
#include "Engine/Interface/Texture.h"
#include "FileSystem/ImageLoader.h"

namespace Maple
{

	TextureAtlas::TextureAtlas(uint32_t w, uint32_t h)
	{
		texture = Texture2D::create();
		texture->buildTexture(TextureFormat::RGBA8, w, h, true, false, false);

		leftovers = QuadTree<size_t, LeftOver>([](const LeftOver& first, const LeftOver& second)
		{
				bool wcomp = first.width() >= second.width();
				bool hcomp = first.height() >= second.height();
				if (wcomp && hcomp)
				{
					return QuadTree<size_t, LeftOver>::RIGHT;
				}
				if (wcomp)
				{
					return QuadTree<size_t, LeftOver>::DOWN;
				}
				if (hcomp)
				{
					return QuadTree<size_t, LeftOver>::UP;
				}
				return QuadTree<size_t, LeftOver>::LEFT;
		});

	}

	auto TextureAtlas::addSprite(const std::string& file) ->Quad2D *
	{
		auto image = ImageCache::get(file);
		if (auto iter = offsets.find(file); iter != offsets.end()) {
			return &iter->second;
		}

		if (image->getWidth() <= 0 || image->getHeight() <= 0)
			return nullptr;

		int16_t x = 0;
		int16_t y = 0;
		int16_t w = static_cast<int16_t>(image->getWidth());
		int16_t h = static_cast<int16_t>(image->getHeight());


		LeftOver value(x, y, w, h);
		size_t lid = leftovers.findNode(value, [](const LeftOver& val, const LeftOver& leaf)
		{
				return val.width() <= leaf.width() && val.height() <= leaf.height();
		});


		if (lid > 0)
		{
			const LeftOver& leftover = leftovers[lid];
			x = leftover.l;
			y = leftover.t;

			auto wdelta = leftover.width() - w;
			auto hdelta = leftover.height() - h;

			leftovers.erase(lid);

			wasted -= w * h;
			if (wdelta >= MINLOSIZE && hdelta >= MINLOSIZE)
			{
				leftovers.add(rlid, LeftOver(x + w, y + h, wdelta, hdelta));
				rlid++;

				if (w >= MINLOSIZE)
				{
					leftovers.add(rlid, LeftOver(x, y + h, w, hdelta));
					rlid++;
				}

				if (h >= MINLOSIZE)
				{
					leftovers.add(rlid, LeftOver(x + w, y, wdelta, h));
					rlid++;
				}
			}
			else if (wdelta >= MINLOSIZE)
			{
				leftovers.add(rlid, LeftOver(x + w, y, wdelta, h + hdelta));
				rlid++;
			}
			else if (hdelta >= MINLOSIZE)
			{
				leftovers.add(rlid, LeftOver(x, y + h, w + wdelta, hdelta));
				rlid++;
			}
		}
		else
		{
			if (border.first + w > texture->getWidth())
			{
				auto prevBorder = border.first;
				border.first = 0;
				border.second += yRange.second;
				if (border.second + h > texture->getHeight())
				{
					border.first = prevBorder;
					border.second -= yRange.second;
					return nullptr;
				}
				yRange = {};
			}
			else if (border.second + h > texture->getHeight())
			{
				return nullptr;
			}
			x = border.first;
			y = border.second;

			border.first += w;
			if (h > yRange.second)
			{
				if (x >= MINLOSIZE && h - yRange.second >= MINLOSIZE)
				{
					leftovers.add(rlid, LeftOver(0, yRange.first, x, h - yRange.second));
					rlid++;
				}
				wasted += x * (h - yRange.second);
				yRange = { y + h, h };
			}
			else if (h < yRange.first - y)
			{
				if (w >= MINLOSIZE && yRange.first - y - h >= MINLOSIZE)
				{
					leftovers.add(rlid, LeftOver(x, y + h, w, yRange.first - y - h));
					rlid++;
				}
				wasted += w * (yRange.first - y - h);
			}
		}

		texture->bind();
		texture->update(x, y, w, h, (const uint8_t*)image->getData());
		auto& offset = offsets[file];
		offset.setTexCoords(x, y, w, h);
		offset.setTexture(texture);
		return &offset;
	}

};