#include "depthstencil.h"

DepthStencil::DepthStencil(const VulkanContext& vkCtx, int width, int height) : _vkCtx(vkCtx) {
	auto info = vk::ImageCreateInfo()
		.setArrayLayers(1)
		.setExtent(vk::Extent3D(width, height, 1))
		.setFormat(vk::Format::eD32Sfloat)
		.setImageType(vk::ImageType::e2D)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
		.setMipLevels(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(vk::ImageTiling::eOptimal);

	_depthImage = vkCtx.getDevice().createImage(info);

	VmaAllocationCreateInfo allocCreateInfo{};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	vmaAllocateMemoryForImage(vkCtx.getAllocator(), _depthImage, &allocCreateInfo, &_allocation, &_allocInfo);
	vmaBindImageMemory(vkCtx.getAllocator(), _allocation, _depthImage);

	auto createInfo = vk::ImageViewCreateInfo()
		.setFormat(vk::Format::eD32Sfloat)
		.setImage(_depthImage)
		.setViewType(vk::ImageViewType::e2D)
		.setSubresourceRange(vk::ImageSubresourceRange()
			.setAspectMask(vk::ImageAspectFlagBits::eDepth)
			.setBaseArrayLayer(0)
			.setLayerCount(1)
			.setBaseMipLevel(0)
			.setLevelCount(1));

	_depthImageView = _vkCtx.getDevice().createImageView(createInfo);
}

DepthStencil::~DepthStencil() {
	vmaDestroyImage(_vkCtx.getAllocator(), _depthImage, _allocation);
}

vk::AttachmentDescription DepthStencil::getDepthAttachment()
{
	return vk::AttachmentDescription()
		.setFormat(vk::Format::eD32Sfloat)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(getLayout());
}

vk::ImageLayout DepthStencil::getLayout()
{
	return vk::ImageLayout::eDepthStencilAttachmentOptimal;
}

vk::ImageView DepthStencil::getImageView()
{
	return _depthImageView;
}
