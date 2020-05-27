#pragma once

#include <SDL2/SDL.h>

#include <vulkan/vulkan.hpp>

#include <optional>
#include <vector>

struct QueueFamilies {
	std::optional<uint32_t> computeInd;
	std::optional <uint32_t> transferInd;
	std::optional <uint32_t> graphicsInd;
};

class VulkanContext
{
	const vk::Instance _instance;
	const vk::PhysicalDevice _physicalDevice;
	const vk::Device _device;

	const QueueFamilies _queueFamilies;
	const vk::DebugUtilsMessengerEXT _debugUtils;

	const size_t _nComputeQueues;
	const size_t _nTransferQueues;
	const size_t _nGraphicsQueues;

	static VulkanContext createContext(SDL_Window* window);
	static VulkanContext createContext(const std::vector<const char*>& extensionNames);
public:
	VulkanContext(SDL_Window* window);
	VulkanContext(vk::Instance instance,
		vk::PhysicalDevice physicalDevice,
		vk::Device device,
		QueueFamilies queueFamilies,
		vk::DebugUtilsMessengerEXT debugUtils,
		size_t nComputeQueues,
		size_t nTransferQueues,
		size_t nGraphicsQueues);
	VulkanContext(VulkanContext&&) = default;
	VulkanContext& operator=(VulkanContext&&) = default;
	~VulkanContext();

	vk::SurfaceKHR createSurfaceFromWindow(SDL_Window* window) const;
	vk::SwapchainKHR createSwapchain(vk::SurfaceKHR surface);
	
	template<typename T>
	void deviceDestroy(T v) const {
		_device.destroy(v);
	}

	vk::Instance getInstance() const;
	vk::PhysicalDevice getPhysicalDevice() const;
	vk::Device getDevice() const;
	QueueFamilies getQueueFamilies() const;

	void destroy(vk::SurfaceKHR &surface) const;
};