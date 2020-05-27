#include "VulkanContext.h"
#include <SDL2/SDL_vulkan.h>
#include <iostream>

#ifdef NDEBUG
constexpr bool debug = false;
#else
constexpr bool debug = true;
#endif


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

vk::PhysicalDevice getSuitableDevice(const std::vector<vk::PhysicalDevice>& physicalDevices) {
	if (physicalDevices.size() == 0) {
		throw std::exception("No suitable devices");
	}
	return physicalDevices[0];
}

QueueFamilies findQueueFamilies(vk::PhysicalDevice physicalDevice) {
	std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
	QueueFamilies families;
	for (uint32_t i = 0; i < queueFamilies.size(); i++) {
		const vk::QueueFamilyProperties& queueProperties = queueFamilies[i];
		if (queueProperties.queueFlags & vk::QueueFlagBits::eGraphics) {
			families.graphicsInd = i;
			continue;
		}
		if (queueProperties.queueFlags & vk::QueueFlagBits::eCompute) {
			families.computeInd = i;
			continue;
		}
		if (queueProperties.queueFlags & vk::QueueFlagBits::eTransfer) {
			families.transferInd = i;
			continue;
		}
	}
	return families;
}

vk::Device createDevice(vk::PhysicalDevice& physicalDevice,
	QueueFamilies& queueFamilies,
	size_t nComputeQueues,
	size_t nTransferQueues,
	size_t nGraphicsQueues) {

	std::vector<float> graphicsQueuePriorites = { 1.0, 1.0 };
	auto graphicsQueueInfo = vk::DeviceQueueCreateInfo()
		.setQueueCount(graphicsQueuePriorites.size())
		.setPQueuePriorities(graphicsQueuePriorites.data())
		.setQueueFamilyIndex(queueFamilies.graphicsInd.value());

	std::vector<float> computeQueuePriorites = { 1.0, 1.0 };
	auto computeQueueInfo = vk::DeviceQueueCreateInfo()
		.setQueueCount(computeQueuePriorites.size())
		.setPQueuePriorities(computeQueuePriorites.data())
		.setQueueFamilyIndex(queueFamilies.computeInd.value());

	std::vector<float> transferQueuePriorites = { 1.0 };
	auto transferQueueInfo = vk::DeviceQueueCreateInfo()
		.setQueueCount(transferQueuePriorites.size())
		.setPQueuePriorities(transferQueuePriorites.data())
		.setQueueFamilyIndex(queueFamilies.transferInd.value());

	std::vector<vk::DeviceQueueCreateInfo> queueInfos = { graphicsQueueInfo, computeQueueInfo, transferQueueInfo };

	vk::PhysicalDeviceFeatures features;

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo()
		.setEnabledLayerCount(validationLayers.size())
		.setPpEnabledLayerNames(validationLayers.data())
		.setEnabledExtensionCount(extensions.size())
		.setPpEnabledExtensionNames(extensions.data())
		.setQueueCreateInfoCount(queueInfos.size())
		.setPQueueCreateInfos(queueInfos.data())
		.setPEnabledFeatures(&features);

	return physicalDevice.createDevice(deviceInfo);
}

vk::Instance createInstance(const std::vector<const char *> &extensionNames) {
	vk::ApplicationInfo appInfo("Test App", 1, "Test Engine", 1, VK_API_VERSION_1_2);

	std::vector<const char*> layerNames;
	if (debug) {
		layerNames = { "VK_LAYER_KHRONOS_validation" };
	}
	else {
		layerNames = {};
	}

	vk::InstanceCreateInfo instanceInfo({}, &appInfo, layerNames.size(), layerNames.data(), extensionNames.size(), extensionNames.data());
	return vk::createInstance(instanceInfo);
}

vk::DebugUtilsMessengerEXT createDebugUtils(vk::Instance& instance) {
	PFN_vkCreateDebugUtilsMessengerEXT createDebug = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	VkDebugUtilsMessengerCreateInfoEXT debugInfo;
	debugInfo = {};
	debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugInfo.pfnUserCallback = debugCallback;


	VkDebugUtilsMessengerEXT debug = {};
	if (createDebug == nullptr) throw std::exception("Could not find debug utils");
	createDebug(instance, &debugInfo, nullptr, &debug);
	return debug;
}

void destroyDebugUtils(vk::Instance instance, vk::DebugUtilsMessengerEXT debug) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debug, nullptr);
	}
}


VulkanContext VulkanContext::createContext(SDL_Window* window)
{
	std::vector<const char*> extensionNames = {};
	if (debug) {
		extensionNames.push_back({ VK_EXT_DEBUG_UTILS_EXTENSION_NAME });
	}

	unsigned count;
	SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
	extensionNames.resize(extensionNames.size() + count);
	SDL_Vulkan_GetInstanceExtensions(window, &count, &*(extensionNames.end() - count));

	return createContext(extensionNames);
}

VulkanContext VulkanContext::createContext(const std::vector<const char*> &extensionNames)
{
	size_t nComputeQueues = 1;
	size_t nTransferQueues = 1;
	size_t nGraphicsQueues = 1;
	vk::Instance instance = createInstance(extensionNames);
	vk::DebugUtilsMessengerEXT debugUtils;
	if (debug) {
		debugUtils = createDebugUtils(instance);
	}
	vk::PhysicalDevice physicalDevice = getSuitableDevice(instance.enumeratePhysicalDevices());
	QueueFamilies queueFamilies = findQueueFamilies(physicalDevice);

	vk::Device device = createDevice(physicalDevice, queueFamilies, nComputeQueues, nTransferQueues, nGraphicsQueues);

	return VulkanContext(instance, physicalDevice, device, queueFamilies, debugUtils, nComputeQueues, nTransferQueues, nGraphicsQueues);
}

VulkanContext::VulkanContext(SDL_Window* window) : VulkanContext(createContext(window))
{
}

VulkanContext::VulkanContext(const vk::Instance instance,
	vk::PhysicalDevice physicalDevice,
	vk::Device device,
	QueueFamilies queueFamilies,
	vk::DebugUtilsMessengerEXT debugUtils,
	size_t nComputeQueues,
	size_t nTransferQueues,
	size_t nGraphicsQueues)
	: _instance(instance),
	_physicalDevice(physicalDevice),
	_device(device),
	_queueFamilies(queueFamilies),
	_debugUtils(debugUtils),
	_nComputeQueues(nComputeQueues),
	_nTransferQueues(nTransferQueues),
	_nGraphicsQueues(nGraphicsQueues)
{
}

VulkanContext::~VulkanContext()
{
	_device.destroy();
	if (debug) destroyDebugUtils(_instance, _debugUtils);
	_instance.destroy();
}

vk::SurfaceKHR VulkanContext::createSurfaceFromWindow(SDL_Window* window) const
{
	VkSurfaceKHR surface;
	if (!SDL_Vulkan_CreateSurface(window, _instance, &surface)) {
		throw std::exception("Could not create surface from window");
	}
	return surface;
}

vk::SwapchainKHR VulkanContext::createSwapchain(vk::SurfaceKHR surface)
{
	return vk::SwapchainKHR();
}

vk::Instance VulkanContext::getInstance() const
{
	return _instance;
}

vk::PhysicalDevice VulkanContext::getPhysicalDevice() const
{
	return _physicalDevice;
}

vk::Device VulkanContext::getDevice() const
{
	return _device;
}

QueueFamilies VulkanContext::getQueueFamilies() const
{
	return _queueFamilies;
}

void VulkanContext::destroy(vk::SurfaceKHR& surface) const
{
	_instance.destroy(surface);
}