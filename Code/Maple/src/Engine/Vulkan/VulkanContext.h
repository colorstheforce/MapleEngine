//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 

////////////////////////////////////////////////////////////////////////////// 

#pragma once
#include <memory>
#include <vector>
#include <optional>
#include <vulkan/vulkan.h>
#include "Engine/Core.h"

struct GLFWwindow;
namespace Maple
{
	class SwapChain;

	class MAPLE_EXPORT VulkanContext
	{
	public:
		VulkanContext(bool enableValidation = false);
		~VulkanContext();

		auto init() -> void;

		auto createSurface(GLFWwindow* win) -> void;
		auto createSwapChain() -> void;
		auto resize(uint32_t width, uint32_t height) -> void;
		auto waiteIdle() -> void;

		inline const auto getVkInstance() const { return vkInstance; }
		inline auto getVkInstance() { return vkInstance; }
		inline auto getVkSurface() { return surface; }
		inline auto& getSwapChain() { return swapChain; }

		inline auto isEnableValidation() { return enableValidation; }
		inline const auto& getValidationLayers() const { return validationLayers; }

		static auto get()->std::shared_ptr<VulkanContext>;
		static auto release()->void;
	private:
		auto setupDebug() -> void;
		auto getRequireExtensions()->std::vector<const char*>;
		static std::shared_ptr<VulkanContext> instance;

		bool enableValidation = false;
		VkInstance vkInstance;
		VkDebugUtilsMessengerEXT debugCallback{};
		std::vector<const char*> validationLayers;
		uint32_t width;
		uint32_t height;
		bool vsync = true;

		VkSurfaceKHR surface = VK_NULL_HANDLE;

		std::shared_ptr<SwapChain> swapChain;

	};

};