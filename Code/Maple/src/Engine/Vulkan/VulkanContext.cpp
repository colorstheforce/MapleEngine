//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 

////////////////////////////////////////////////////////////////////////////// 

#include "VulkanContext.h"
#include "VulkanHelper.h"
#include "VulkanDevice.h"
#include "Others/Console.h"
#include "Application.h"
#ifdef _WIN32
#include <vulkan/vulkan_win32.h>
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#endif

#include "VulkanSwapChain.h"
namespace Maple
{

	VulkanContext::VulkanContext(bool validation)
		:enableValidation(validation)
	{
		validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};
	}

	VulkanContext::~VulkanContext()
	
	{
		
		vkDestroySurfaceKHR(vkInstance, surface, nullptr);
		if (enableValidation) {
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr) {
				func(vkInstance, debugCallback, nullptr);
			}
		}
		vkDestroyInstance(vkInstance, nullptr);
	}
	/**
	 * setup debug layer
	 */
	auto VulkanContext::setupDebug() -> void
	{
		if (!enableValidation) return;
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		VulkanHelper::populateDebugMessengerCreateInfo(createInfo);
		auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(vkInstance, "vkCreateDebugUtilsMessengerEXT"));
		if (func == nullptr || func(vkInstance, &createInfo, nullptr, &debugCallback) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}
	/**
	 * init the context
	 */
	auto VulkanContext::init() -> void
	{
		if (enableValidation && !VulkanHelper::checkValidationLayerSupport(validationLayers)) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		const std::string name = "SampleApp";
		VkApplicationInfo appInfo = VulkanHelper::getApplicationInfo(name);
		auto extensions = getRequireExtensions();


		auto instanceCreateInfo = VulkanHelper::instanceCreateInfo(appInfo, extensions, validationLayers, enableValidation);


		if (enableValidation) 
		{
			VkDebugReportCallbackCreateInfoEXT debugCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };

			debugCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT 
				| VK_DEBUG_REPORT_WARNING_BIT_EXT 
				| VK_DEBUG_REPORT_INFORMATION_BIT_EXT 
				| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT 
				| VK_DEBUG_REPORT_DEBUG_BIT_EXT;
			debugCreateInfo.pfnCallback = [](
				VkDebugReportFlagsEXT                       flags,
				VkDebugReportObjectTypeEXT                  objectType,
				uint64_t                                    object,
				size_t                                      location,
				int32_t                                     messageCode,
				const char* pLayerPrefix,
				const char* pMessage,
				void* pUserData) -> VkBool32 {

					LOGV("{0}", pMessage);

					return 0;
			};

			instanceCreateInfo.pNext = &debugCreateInfo;

			VkValidationFeatureEnableEXT enabled[] = { VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT };
			VkValidationFeaturesEXT      features{ VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT };
			features.disabledValidationFeatureCount = 0;
			features.enabledValidationFeatureCount = 1;
			features.pDisabledValidationFeatures = nullptr;
			features.pEnabledValidationFeatures = enabled;

			features.pNext = instanceCreateInfo.pNext;
			instanceCreateInfo.pNext = &features;

		}
		VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance));
		setupDebug();
	}
	/**
	 * create surface 
	 */
	auto VulkanContext::createSurface(GLFWwindow* win) -> void
	{
		if (glfwCreateWindowSurface(vkInstance, win, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	/**
	 * create the swapchain
	 */
	auto VulkanContext::createSwapChain() -> void
	{
		swapChain = SwapChain::create();
		swapChain->init();
	}

	/**
	 * if the window size changed, the swapChain should be recreate.
	 */
	auto VulkanContext::resize(uint32_t width, uint32_t height) -> void
	{
		swapChain = SwapChain::create();
		swapChain->init();
	}

	auto VulkanContext::waiteIdle() -> void
	{
		vkDeviceWaitIdle(*VulkanDevice::get());
	}

	auto VulkanContext::get() ->std::shared_ptr<VulkanContext>
	{
		if (instance == nullptr) {
			instance = std::make_shared<VulkanContext>(false);
		}
		return instance;
	}

	auto VulkanContext::release() ->void
	{
		if (instance) {
			instance.reset();
		}
	}

	auto VulkanContext::getRequireExtensions() ->std::vector<const char*>
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (enableValidation) {
			extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;
	}

	std::shared_ptr<VulkanContext> VulkanContext::instance;

};