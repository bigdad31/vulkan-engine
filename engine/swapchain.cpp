#define NOMINMAX

#include "swapchain.h"

#include <algorithm>

Swapchain Swapchain::createSwapchain(const VulkanContext& vkCtx, vk::SurfaceKHR surface, uint32_t width, uint32_t height)
{
	if (vkCtx.getPhysicalDevice().getSurfaceSupportKHR(vkCtx.getQueueFamilies().graphicsInd.value(), surface) != true) {
		throw std::exception("Cannot present to surface");
	}

	vk::SurfaceCapabilitiesKHR capabilities = vkCtx.getPhysicalDevice().getSurfaceCapabilitiesKHR(surface);
	std::vector<vk::SurfaceFormatKHR> formats = vkCtx.getPhysicalDevice().getSurfaceFormatsKHR(surface);
	std::vector<vk::PresentModeKHR> presentModes = vkCtx.getPhysicalDevice().getSurfacePresentModesKHR(surface);

	auto formatIter = std::find_if(formats.begin(), formats.end(), [](vk::SurfaceFormatKHR format) {
		return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; 
	});

	vk::SurfaceFormatKHR format = formats[0];
	if (formatIter != formats.end()) {
		format = *formatIter;
	}

	auto presentModeIter = std::find(presentModes.begin(), presentModes.end(), vk::PresentModeKHR::eMailbox);
	vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
	if (presentModeIter != presentModes.end()) {
		presentMode = *presentModeIter;
	}
	vk::Extent2D extent = capabilities.currentExtent;

	if (capabilities.currentExtent.width == UINT32_MAX) {
		extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, width));
		extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, height));
	}

	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
		imageCount = capabilities.maxImageCount;
	}

	auto swapchainCreateInfo = vk::SwapchainCreateInfoKHR()
		.setSurface(surface)
		.setImageFormat(format.format)
		.setImageColorSpace(format.colorSpace)
		.setImageExtent(extent)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		.setPresentMode(presentMode)
		.setMinImageCount(imageCount)
		.setImageArrayLayers(1)
		.setPreTransform(capabilities.currentTransform)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setClipped(true)
		.setImageSharingMode(vk::SharingMode::eExclusive);

	auto swapchain = vkCtx.getDevice().createSwapchainKHR(swapchainCreateInfo);
	std::vector<vk::Image> images = vkCtx.getDevice().getSwapchainImagesKHR(swapchain);
	std::vector<vk::ImageView> imageViews(images.size());
	std::transform(images.begin(), images.end(), imageViews.begin(), [&](vk::Image& image) {
		auto imageViewCreateInfo = vk::ImageViewCreateInfo()
			.setImage(image)
			.setViewType(vk::ImageViewType::e2D)
			.setComponents(vk::ComponentMapping())
			.setFormat(format.format)
			.setSubresourceRange(vk::ImageSubresourceRange()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseArrayLayer(0)
				.setLayerCount(1)
				.setBaseMipLevel(0)
				.setLevelCount(1));
		return vkCtx.getDevice().createImageView(imageViewCreateInfo);
	});

	return Swapchain(vkCtx, swapchain, images, imageViews, surface, extent, format);
}

Swapchain::Swapchain(const VulkanContext& vkCtx, vk::SurfaceKHR surface, uint32_t width, uint32_t height)
	: Swapchain(createSwapchain(vkCtx, surface, width, height))
{
}

Swapchain::Swapchain(const VulkanContext& vkCtx, vk::SwapchainKHR swapchain, std::vector<vk::Image> images, std::vector<vk::ImageView> imageViews, vk::SurfaceKHR surface, vk::Extent2D extent, vk::SurfaceFormatKHR format)
	: _vkCtx(vkCtx),
	_swapchain(swapchain),
	_images(images),
	_imageViews(imageViews),
	_surface(surface),
	_extent(extent),
	_format(format)
{
}

Swapchain::~Swapchain()
{
	for (const auto& imageView : _imageViews) {
		_vkCtx.getDevice().destroy(imageView);
	}
	_vkCtx.getDevice().destroy(_swapchain);
}

int Swapchain::getWidth() const
{
	return _extent.width;
}

int Swapchain::getHeight() const
{
	return _extent.height;
}

vk::Viewport Swapchain::getViewport() const
{
	return vk::Viewport()
		.setX(0)
		.setY(0)
		.setWidth(getWidth())
		.setHeight(getHeight())
		.setMinDepth(0)
		.setMaxDepth(1.0);
}

vk::Rect2D Swapchain::getScissors() const
{
	return vk::Rect2D()
		.setOffset({ 0, 0 })
		.setExtent(_extent);
}

vk::SurfaceFormatKHR Swapchain::getFormat() const
{
	return _format;
}

const std::vector<vk::ImageView>& Swapchain::getImageViews() const
{
	return _imageViews;
}
