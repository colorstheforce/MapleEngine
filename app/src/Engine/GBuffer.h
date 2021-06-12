
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <glm/glm.hpp>
#include <array>
#include <memory>
#include "Engine/Interface/Texture.h"

namespace Maple
{

	enum class TextureFormat;
	enum GBufferTextures
	{
		DEPTH = 0,		//Depth Buffer
		COLOR = 1,		//Main Render
		POSITION = 2,	//Deferred Render - World Space Positions
		NORMALS = 3,	//Deferred Render - World Space Normals
		PBR = 4,	
		OFFSCREEN0 = 5,	
		OFFSCREEN1 = 6,   
		LENGTH
	};

	class GBuffer
	{
	public:
		GBuffer(uint32_t width, uint32_t height);

		inline auto getWidth() const { return width; }
		inline auto getHeight() const { return height; }
		auto resize(uint32_t width, uint32_t height) -> void;
		auto buildTexture() -> void;
		inline auto getDepthBuffer() { return depthBuffer; }
		inline auto getBuffer(uint32_t index) { return screenTextures[index]; }
		inline auto getFormat(uint32_t index) { return formats[index]; }
		
		static auto getGBufferTextureName(GBufferTextures index) -> const char*;
	private:
		std::array<std::shared_ptr<Texture2D>, GBufferTextures::LENGTH> screenTextures;
		std::array<TextureFormat, GBufferTextures::LENGTH> formats;
		std::shared_ptr<TextureDepth>  depthBuffer;
	private:
		uint32_t width;
		uint32_t height;
	};
}
