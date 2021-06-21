//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 

////////////////////////////////////////////////////////////////////////////// 

#pragma once
#include <string>
#include <memory>

namespace Maple
{
	enum class TextureWrap
	{
		NONE,
		REPEAT,
		CLAMP,
		MIRRORED_REPEAT,
		CLAMP_TO_EDGE,
		CLAMP_TO_BORDER
	};

	enum class TextureFilter
	{
		NONE,
		LINEAR,
		NEAREST,
		LINEAR_MIPMAP_LINEAR
	};

	enum class TextureFormat
	{
		NONE,
		R8,
		RG8,
		RGB8,
		RGBA8,
		RGB16,
		RGBA16,
		RGB32,
		RGBA32,
		RGB,
		RGBA,
		BGRA8,
		DEPTH,
		STENCIL,
		DEPTH_STENCIL
	};

	enum class TextureType
	{
		COLOR,
		DEPTH,
		DEPTHARRAY,
		CUBE,
		OTHER
	};

	struct TextureParameters
	{
		TextureFormat format;
		TextureFilter minFilter;
		TextureFilter magFilter;
		TextureWrap wrap;
		bool srgb;
		TextureParameters()
		{
			format = TextureFormat::RGBA;
			minFilter = TextureFilter::LINEAR;
			magFilter = TextureFilter::LINEAR;
			wrap = TextureWrap::REPEAT;
			srgb = false;
		}

		TextureParameters(TextureFormat format, TextureFilter minFilter, TextureFilter magFilter, TextureWrap wrap)
			: format(format)
			, minFilter(minFilter)
			, magFilter(magFilter)
			, wrap(wrap)
			, srgb(false)
		{
		}

		TextureParameters(TextureFilter minFilter, TextureFilter magFilter)
			: format(TextureFormat::RGBA)
			, minFilter(minFilter)
			, magFilter(magFilter)
			, wrap(TextureWrap::CLAMP), srgb(false)
		{
		}

		TextureParameters(TextureFilter minFilter, TextureFilter magFilter, TextureWrap wrap)
			: format(TextureFormat::RGBA)
			, minFilter(minFilter)
			, magFilter(magFilter)
			, wrap(wrap), srgb(false)
		{
		}

		TextureParameters(TextureWrap wrap)
			: format(TextureFormat::RGBA)
			, minFilter(TextureFilter::LINEAR)
			, magFilter(TextureFilter::LINEAR)
			, wrap(wrap), srgb(false)
		{
		}

		TextureParameters(TextureFormat format)
			: format(format)
			, minFilter(TextureFilter::LINEAR)
			, magFilter(TextureFilter::LINEAR)
			, wrap(TextureWrap::CLAMP), srgb(false)
		{
		}
	};

	struct TextureLoadOptions
	{
		bool flipX;
		bool flipY;
		bool generateMipMaps;

		TextureLoadOptions()
		{
			flipX = false;
			flipY = false;
			generateMipMaps = true;
		}

		TextureLoadOptions(bool flipX, bool flipY, bool genMips = true)
			: flipX(flipX)
			, flipY(flipY)
			, generateMipMaps(genMips)
		{
		}
	};
};