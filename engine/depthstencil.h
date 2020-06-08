#pragma once
#include "vulkancontext.h"

class DepthStencil
{
	const VulkanContext& _vkCtx;
	VmaAllocation _allocation{};
	VmaAllocationInfo _allocInfo{};
	vk::Image _depthImage;
	vk::ImageView _depthImageView;

public:
	DepthStencil(const VulkanContext& vkCtx, float width, float height);
	~DepthStencil();

	static vk::AttachmentDescription getDepthAttachment();
	static vk::ImageLayout getLayout();

	vk::ImageView getImageView();
};
