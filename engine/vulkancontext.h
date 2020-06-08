#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <SDL2/SDL_video.h>

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

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
	const VmaAllocator _allocator;

	const QueueFamilies _queueFamilies;
	const vk::DebugUtilsMessengerEXT _debugUtils;

	const std::vector<vk::Queue> _computeQueues;
	const std::vector<vk::Queue> _transferQueues;
	const std::vector<vk::Queue> _graphicsQueues;

	static VulkanContext createContext(SDL_Window* window);
	static VulkanContext createContext(const std::vector<const char*>& extensionNames);
public:
	VulkanContext(SDL_Window* window);
	VulkanContext(const vk::Instance instance,

		vk::PhysicalDevice physicalDevice,

		vk::Device device, VmaAllocator allocator,

		QueueFamilies queueFamilies,

		vk::DebugUtilsMessengerEXT debugUtils,

		std::vector<vk::Queue> computeQueues,
		std::vector<vk::Queue> transferQueues,
		std::vector<vk::Queue> graphicsQueues);
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
	VmaAllocator getAllocator() const
	{
		return _allocator;
	}
	QueueFamilies getQueueFamilies() const;

	vk::Queue getComputeQueue(int i) const {
		return _computeQueues[i];
	}

	vk::Queue getGraphicsQueue(int i) const {
		return _graphicsQueues[i];
	}

	vk::Queue getTransferQueue(int i) const {
		return _transferQueues[i];
	}

	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const {
		vk::PhysicalDeviceMemoryProperties memProperties = _physicalDevice.getMemoryProperties();

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}


	void destroySurface(vk::SurfaceKHR surface) const;
};

template<class T, size_t minPadding = 0>
struct Buffer {
	vk::Buffer data;
	VmaAllocation allocation;
	size_t size;

	Buffer() = default;
	Buffer(const VulkanContext &vkCtx, size_t size, vk::BufferUsageFlags usage, VmaMemoryUsage memory = VMA_MEMORY_USAGE_GPU_ONLY) {
		this->size = size;
		auto info = vk::BufferCreateInfo()
			.setUsage(usage)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setSize(getMinSize() * size);

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = memory;

		vmaCreateBuffer(vkCtx.getAllocator(), (VkBufferCreateInfo*)(&info), &allocCreateInfo, (VkBuffer*)&data, &allocation, nullptr);
	}

	size_t getMinSize() {
		return (minPadding > sizeof(T)?minPadding: sizeof(T));
	}

	void destroy(const VulkanContext& vkCtx) {
		vmaDestroyBuffer(vkCtx.getAllocator(), data, allocation);
	}
};