//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "TerrainBuilder.h"
#include "FileSystem/ImageLoader.h"
#include <cmath>

namespace Maple 
{
	struct color8888
	{
		uint8_t r : 8;
		uint8_t g : 8;
		uint8_t b : 8;
		uint8_t a : 8;
	};


	TerrainBuilder::TerrainBuilder(const std::string& filePath) 
		:name(filePath)
	{
		heightMap = ImageLoader::loadAsset(filePath);
	}

	auto TerrainBuilder::build() -> std::shared_ptr<QuadCollapseMesh>
	{
		auto buffer = reinterpret_cast<const color8888*>(heightMap->getData());

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<glm::vec4> colors;

		auto minY = 255;
		auto maxY = -255;

		for (int32_t x = 0; x < heightMap->getWidth(); x++)
		{
			for (int32_t y = 0; y < heightMap->getHeight(); y++)
			{
				const auto & pixelColor= buffer[y * heightMap->getWidth() + x];
				float height = pixelColor.r;
				vertices.emplace_back(Vertex{});
				vertices.back().pos = { x , y , height /*/ 255.f  * 10*/};
				vertices.back().color = { pixelColor.r,pixelColor.g, pixelColor.b, pixelColor.a };
				vertices.back().texCoord = { x / ((float)heightMap->getWidth() - 1.f), y / ((float)heightMap->getHeight() - 1.f) };
				if (height < minY) 
				{
					minY = height;
				}
				if (height >  maxY)
				{
					maxY = height;
				}
			}
		}
	
		auto height = [&](int32_t x,int32_t y) {
			if (x < 0 || y < 0 || x >= heightMap->getWidth() || y >= heightMap->getHeight()) {
				return 0.f;
			}
			return vertices[x + y * heightMap->getWidth()].pos.z / 255.f;
		};

		for (auto & v : vertices)
		{
			float hL = height(v.pos.x - 1, v.pos.y);
			float hR = height(v.pos.x + 1, v.pos.y);

			float hD = height(v.pos.x, v.pos.y - 1);
			float hU = height(v.pos.x, v.pos.y + 1);

			//Central Differencing
			v.normal.x = hL - hR;
			v.normal.y = hD - hU;
			v.normal.z = 2.0;
			v.normal = glm::normalize(v.normal);
		}


		//auto vb = std::make_shared<VertexBuffer>();
		//vb->setData(sizeof(vertices[0]) * vertices.size(), vertices.data());
		//auto ib = std::make_shared<IndexBuffer>(indices.data(),indices.size());
		//auto terr = std::make_shared<Terrain>(vb, ib);
		//terr->setIndicesSize(indices.size());
		//terr->setTexture(Texture2D::create("default", "textures/terrain/rock.jpg"));
		auto terr = std::make_shared<QuadCollapseMesh>();
		terr->build(vertices, heightMap->getWidth(), heightMap->getHeight());
		terr->heightMap = Texture2D::create("height", name);
		return terr;

	}
	
};
