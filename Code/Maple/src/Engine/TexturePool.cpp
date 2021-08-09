#include "TexturePool.h"

namespace Maple
{
    auto TexturePool::addSprite(const std::string& file) -> Quad2D*
    {
        if (auto iter = mapping.find(file); iter != mapping.end()) {
            return iter->second->addSprite(file);
        }
        if (atlas.empty()) {
            createTextureAtlas();
        }
		auto quad = atlas.back().addSprite(file);
        if (quad == nullptr) {
            auto txt = createTextureAtlas();
            quad = txt->addSprite(file);
        }
		mapping[file] = &atlas.back();
		return quad;
    }

    auto TexturePool::addSprite(const std::string& uniqueName, const std::vector<uint8_t>& data, uint32_t w, uint32_t h) -> Quad2D*
    {
		if (auto iter = mapping.find(uniqueName); iter != mapping.end()) {
			return iter->second->addSprite(uniqueName);
		}

		if (atlas.empty()) {
			createTextureAtlas();
		}
		auto quad = atlas.back().addSprite(uniqueName, data,w,h);
		if (quad == nullptr) {//full.. create a new one
			auto txt = createTextureAtlas();
			quad = txt->addSprite(uniqueName);
		}
		mapping[uniqueName] = &atlas.back();
		return quad;
    }

    auto TexturePool::createTextureAtlas() -> TextureAtlas*
    {
        return &atlas.emplace_back();
    }
};