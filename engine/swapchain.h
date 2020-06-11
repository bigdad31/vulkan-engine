#pragma once

#include "vulkancontext.h"


class Swapchain
{
	const VulkanContext& _vkCtx;
	const vk::SwapchainKHR _swapchain;
	const std::vector<vk::Image> _images;
	const std::vector<vk::ImageView> _imageViews;
	const vk::SurfaceKHR _surface;
	const vk::Extent2D _extent;
	const vk::SurfaceFormatKHR _format;

	static Swapchain createSwapchain(const VulkanContext& vkCtx, vk::SurfaceKHR surface, uint32_t width, uint32_t height);
public:
	Swapchain(const VulkanContext& vkCtx, vk::SurfaceKHR surface, uint32_t width, uint32_t height);
	Swapchain(const VulkanContext& vkCtx, vk::SwapchainKHR swapchain, std::vector<vk::Image> images, std::vector<vk::ImageView> imageViews, vk::SurfaceKHR surface, vk::Extent2D extent, vk::SurfaceFormatKHR format);
	~Swapchain();
	Swapchain(Swapchain&&) = default;

	int getWidth() const;
	int getHeight() const;

	vk::Viewport getViewport() const;
	vk::Rect2D getScissors() const;
	vk::SurfaceFormatKHR getFormat() const;
	vk::SwapchainKHR getSwapchain() const;
	const std::vector<vk::ImageView>& getImageViews() const;
	uint32_t getImageCount() const;

	uint32_t acquireNextImage(vk::Semaphore imageAvailible) const;
};
