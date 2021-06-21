//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 
////////////////////////////////////////////////////////////////////////////// 

#pragma once
#include "Engine/Renderer/RenderParam.h"
#include "Engine/TextureFormat.h"
#include "ktx.h"
#include <memory>
#include <string>

namespace Maple
{
	class FrameBuffer;
	class CommandBuffer;

	class Texture {
	public:
		virtual ~Texture() = default;
		inline auto getWidth() const { return width; }
		inline auto getHeight() const { return height; }

		virtual auto getHandle() const -> void* = 0;
		virtual auto bind(uint32_t slot = 0) const -> void	 = 0 ;
		virtual auto unbind(uint32_t slot = 0) const -> void = 0 ;
	protected:
		uint32_t width = 0;
		uint32_t height = 0;
	};

	class Texture2D : public Texture {
	public:
		static auto getDefaultTexture()->std::shared_ptr<Texture2D>;
		static auto create(const std::string& name, const std::string& fileName)->std::shared_ptr<Texture2D>;
		static auto create(const std::string& fileName, TextureParameters parameters = TextureParameters(), TextureLoadOptions loadOptions = TextureLoadOptions())->std::shared_ptr<Texture2D>;
		static auto create()->std::shared_ptr<Texture2D>;
		static auto createFromSource(uint32_t w, uint32_t h, const uint8_t* data)->std::shared_ptr<Texture2D>;
		

		virtual auto buildTexture(TextureFormat internalformat, uint32_t width, uint32_t height, bool srgb, bool depth, bool samplerShadow) ->void = 0;
		auto loadKTXFile(std::string filename, ktxTexture** target)->ktxResult;
		inline auto getFilePath() const -> const std::string& { return fileName; };
		inline auto getMipmapLevel() const { return mipLevels; }
	protected:
		std::string fileName;
		uint32_t mipLevels = 1;
	};

	class TextureDepth : public Texture
	{
	public:
		static auto create(uint32_t width, uint32_t height)->std::shared_ptr<TextureDepth>;
		virtual auto resize(uint32_t width, uint32_t height) -> void = 0;
	};

	class TextureCube : public Texture
	{
	public:
		enum class InputFormat
		{
			VERTICAL_CROSS,
			HORIZONTAL_CROSS
		};
		static auto create(const std::string& files)->std::shared_ptr<TextureCube>;
		static auto create(int32_t size,TextureFormat format = TextureFormat::RGBA,int32_t numMips = 1)->std::shared_ptr<TextureCube>;

		inline const auto& getTextureParameters() const { return parameters; }
		inline auto& getTextureParameters() { return parameters; }
		inline auto& getMipLevel() const { return numMips; }
		virtual auto update(CommandBuffer* commandBuffer, FrameBuffer* framebuffer, int32_t cubeIndex, int32_t mipmapLevel = 0) -> void;
	protected:
		TextureParameters parameters;
		TextureLoadOptions loadOptions;
		int32_t numMips = 1;
	};


	class TextureDepthArray : public Texture
	{
	public:
		static auto create(uint32_t width, uint32_t height, uint32_t count)->std::shared_ptr<TextureDepthArray>;
		virtual auto init() -> void = 0;
		virtual auto resize(uint32_t width, uint32_t height, uint32_t count) -> void = 0;
		virtual auto getHandleArray(uint32_t index) -> void*{ return getHandle();}
	};

};