//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 

////////////////////////////////////////////////////////////////////////////// 
#include <stdexcept>
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "VulkanHelper.h"
#include "VulkanCommandPool.h"


namespace Maple
{
	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
		VK_KHR_MAINTENANCE1_EXTENSION_NAME
		
	};

	auto VulkanDevice::init() -> bool
	{
		physicalDevice = std::make_shared<VulkanPhysicalDevice>();

		auto indices = VulkanHelper::findQueueFamilies(*physicalDevice, VulkanContext::get()->getVkSurface());

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.emplace_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (VulkanContext::get()->isEnableValidation()) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(VulkanContext::get()->getValidationLayers().size());
			createInfo.ppEnabledLayerNames = VulkanContext::get()->getValidationLayers().data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(*physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

		commandPool = std::make_shared<VulkanCommandPool>();

		createPipelineCache();

		return true;
	}

	VulkanDevice::VulkanDevice()
	{

	}



	VulkanPhysicalDevice::VulkanPhysicalDevice()
	{
		/**
		 * Select a supported graphics card (VkPhysicalDevice)
		 */
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(VulkanContext::get()->getVkInstance(), &deviceCount, nullptr);
		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(VulkanContext::get()->getVkInstance(), &deviceCount, devices.data());

		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				//get params 
				vkGetPhysicalDeviceProperties(physicalDevice, &properties);
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	/**
	 * find a suitable graphics card
	 */
	auto VulkanPhysicalDevice::isDeviceSuitable(VkPhysicalDevice device) -> bool
	{
		indices = VulkanHelper::findQueueFamilies(device, VulkanContext::get()->getVkSurface());
		bool extensionsSupported = VulkanHelper::checkDeviceExtensionSupport(device, deviceExtensions);
		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = VulkanHelper::querySwapChainSupport(device, VulkanContext::get()->getVkSurface());
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}
		VkPhysicalDeviceFeatures supportedFeatures;
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
		return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}


	auto VulkanDevice::get() ->std::shared_ptr<VulkanDevice>
	{
		if (instance == nullptr) {
			instance = std::make_shared<VulkanDevice>();
		}
		return instance;
	}

	auto VulkanDevice::release() -> void
	{
		instance.reset();
	}

	VulkanDevice::~VulkanDevice()
	{
		commandPool.reset();
		if (device != nullptr)
			vkDestroyDevice(device, nullptr);
	}

	auto VulkanDevice::createPipelineCache() -> void
	{
		VkPipelineCacheCreateInfo pipelineCacheCI{};
		pipelineCacheCI.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		pipelineCacheCI.pNext = NULL;
		vkCreatePipelineCache(device, &pipelineCacheCI, VK_NULL_HANDLE, &pipelineCache);
	}

	std::shared_ptr<VulkanDevice> VulkanDevice::instance;
};