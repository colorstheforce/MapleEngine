//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Mesh.h"


namespace Maple 
{
	class Terrain : public Mesh
	{
	public:
		Terrain(int32_t width = 500, int32_t height = 500, int32_t lowSide = 50, int32_t lowScale = 10, float xRand = 1.0f, float yRand = 150.0f, float zRand = 1.0f, float texRandX = 1.0f / 16.0f, float texRandZ = 1.0f / 16.0f);
		Terrain(const std::shared_ptr<VertexBuffer>& vertexBuffer, const std::shared_ptr<IndexBuffer>& indexBuffer);
		auto onUpdate(const Timestep& time, Camera* camera) -> void;
	};
};