//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 

////////////////////////////////////////////////////////////////////////////// 

#pragma once
#include "VulkanHelper.h"
#include "Engine/Interface/Texture.h"

namespace Maple
{

	class VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(uint32_t width, uint32_t height, const void* data, TextureParameters parameters = TextureParameters(), TextureLoadOptions loadOptions = TextureLoadOptions());
		VulkanTexture2D(const std::string& name, const std::string& filename, TextureParameters parameters = TextureParameters(), TextureLoadOptions loadOptions = TextureLoadOptions());
		VulkanTexture2D(VkImage image, VkImageView imageView);
		VulkanTexture2D();
		~VulkanTexture2D();

		auto update(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint8_t* data) -> void override;

		auto bind(uint32_t slot = 0) const -> void override {}
		auto unbind(uint32_t slot = 0) const -> void override {}

		inline auto setData(const void* pixels) -> void {};
		inline auto getHandle() const ->  void* { return (void*)&descriptor; };
	
		inline auto& getName() const { return name; };
	
		inline auto setName(const std::string& name) -> void { this->name = name; }

		inline const auto getDescriptor() const { return &descriptor; }
		inline auto getImage() const { return textureImage; }
		inline auto getDeviceMemory() const { return textureImageMemory; }
		inline auto getImageView() const { return textureImageView; }
		inline auto getSampler() const { return textureSampler; }
		inline auto getImageLayout() const { return imageLayout; }
		auto loadKTX() -> void;

		auto load() -> bool;
		auto updateDescriptor() -> void;
		auto buildTexture(TextureFormat internalformat, uint32_t width, uint32_t height, bool srgb, bool depth, bool samplerShadow) ->void override;
	private:
		std::string name;
	
		uint32_t handle = 0;

		const uint8_t* data = nullptr;

		TextureParameters parameters;
		TextureLoadOptions loadOptions;

		VkImage textureImage = VK_NULL_HANDLE;
		VkImageView textureImageView = VK_NULL_HANDLE;

		VkImageLayout imageLayout;
		VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
		VkSampler textureSampler = VK_NULL_HANDLE;
		VkDescriptorImageInfo descriptor{};

		bool deleteImage = false;

		uint32_t layerCount = 1;
	};



	class VulkanTextureDepth : public TextureDepth
	{
	public:
		VulkanTextureDepth(uint32_t width, uint32_t height);
		~VulkanTextureDepth();

		auto bind(uint32_t slot = 0) const -> void override {}
		auto unbind(uint32_t slot = 0) const -> void override {}
		auto resize(uint32_t width, uint32_t height) -> void;

		virtual auto getHandle() const -> void*
		{
			return (void*)&descriptor;
		}

		inline auto getName() const -> const std::string&
		{
			return name;
		}
		inline auto getFilePath() const -> const std::string&
		{
			return name;
		}

		inline const auto getImage() const
		{
			return textureImage;
		};
		inline const auto getDeviceMemory() const
		{
			return textureImageMemory;
		}
		inline const auto getImageView() const
		{
			return textureImageView;
		}
		inline const auto getSampler() const
		{
			return textureSampler;
		}
		inline const auto getDescriptor() const
		{
			return &descriptor;
		}
		auto updateDescriptor() -> void;


	protected:
		auto init() -> void;
		auto release() -> void;
	private:
		std::string name;
		uint32_t handle{};


		VkImage textureImage{};
		VkDeviceMemory textureImageMemory{};
		VkImageView textureImageView{};
		VkSampler textureSampler{};
		VkDescriptorImageInfo descriptor{};
	};



	class VulkanTextureCube : public TextureCube
	{
	public:
		VulkanTextureCube(int32_t size,TextureFormat format = TextureFormat::RGBA8,int32_t numMips = 1);

		VulkanTextureCube(const std::string& filePath);
		~VulkanTextureCube();
		auto bind(uint32_t slot = 0) const -> void override {}
		auto unbind(uint32_t slot = 0) const -> void override {}

		virtual auto getHandle() const -> void* override { return (void*)&descriptor; }
		auto updateDescriptor() -> void;
		auto load(uint32_t mips) -> void;

		auto update(CommandBuffer* commandBuffer,FrameBuffer * framebuffer,int32_t cubeIndex, int32_t mipmapLevel = 0) -> void override;

		inline auto getImage() const
		{
			return textureImage;
		};
		inline auto getDeviceMemory() const
		{
			return textureImageMemory;
		}
		inline auto getImageView() const
		{
			return textureImageView;
		}
		inline auto getSampler() const
		{
			return textureSampler;
		}



	private:
		auto init() -> void;
	

		std::string name;
		std::string files[6];

		uint32_t handle = 0;
/*
		uint32_t width = 0; 
		uint32_t height = 0; */
		uint32_t size = 0;
		//uint32_t numMips = 0;
		std::vector<uint8_t> data;

		

		VkImage textureImage = nullptr;
		VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkDeviceMemory textureImageMemory = nullptr;
		VkImageView textureImageView = nullptr;
		VkSampler textureSampler;
		VkDescriptorImageInfo descriptor;
	
		bool deleteImg = true;
	};



	class VulkanTextureDepthArray : public TextureDepthArray 
	{
	public:
		VulkanTextureDepthArray(uint32_t width, uint32_t height, uint32_t count);
		~VulkanTextureDepthArray();

		auto bind(uint32_t slot = 0) const -> void override {};
		auto unbind(uint32_t slot = 0) const -> void override {};
		auto resize(uint32_t width, uint32_t height, uint32_t count) -> void override;

		auto getHandle() const -> void* override {return (void*)&descriptor;}
		inline auto getImage() const { return textureImage; }
		inline auto getImageView(int32_t index) const	{	return imageViews[index];	}
		inline auto getSampler() const	{	return textureSampler;	}
		inline auto getDescriptor() const{return &descriptor;}

		auto getHandleArray(uint32_t index) -> void* override;
		auto updateDescriptor() -> void;


	protected:
		auto init() -> void override;
	private:
		uint32_t handle{};
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t count = 0;

		VkImage textureImage{};
		VkDeviceMemory textureImageMemory{};
		VkImageView textureImageView{};
		VkSampler textureSampler{};
		VkDescriptorImageInfo descriptor{};
		std::vector<VkImageView> imageViews;
	};

};