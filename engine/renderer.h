#pragma once

#include "pipeline.h"
#include "drawcommand.h"

constexpr int maxFramesInFlight = 2;

class Renderer
{
	const VulkanContext& _vkCtx;
	vk::SurfaceKHR _surface;
	Swapchain _swapchain;
	vk::Image _depthImage;
	vk::ImageView _depthImageView;
	Pipeline _pipeline;
	DrawCommand _drawCommand;
	TransferHandler _transferHandler;

	std::vector<vk::UniqueHandle<vk::Semaphore, vk::DispatchLoaderStatic>> _imageAvailableSemaphores;
	std::vector<vk::UniqueHandle<vk::Semaphore, vk::DispatchLoaderStatic>> _renderFinishedSemaphores;
	std::vector<vk::UniqueHandle<vk::Fence, vk::DispatchLoaderStatic>> _inFlightFences;
	std::vector<BakedModel> _bakedModels;
	size_t _frame = 0;

	static vk::Image createDepthImage(const VulkanContext& vkCtx, const Swapchain& swapchain) {
		auto info = vk::ImageCreateInfo()
			.setArrayLayers(1)
			.setExtent(vk::Extent3D(swapchain.getWidth(), swapchain.getHeight(), 1))
			.setFormat(vk::Format::eD32Sfloat)
			.setImageType(vk::ImageType::e2D)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
			.setMipLevels(1)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setTiling(vk::ImageTiling::eOptimal);
		
		vk::Image image = vkCtx.getDevice().createImage(info);

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		VmaAllocationInfo allocInfo;
		VmaAllocation allocation;

		vmaAllocateMemoryForImage(vkCtx.getAllocator(), image, &allocCreateInfo, &allocation, &allocInfo);
		vmaBindImageMemory(vkCtx.getAllocator(), allocation, image);

		return image;
	}

	static vk::ImageView createDepthImageView(const VulkanContext &vkCtx,vk::Image image) {
		auto createInfo = vk::ImageViewCreateInfo()
			.setFormat(vk::Format::eD32Sfloat)
			.setImage(image)
			.setViewType(vk::ImageViewType::e2D)
			.setSubresourceRange(vk::ImageSubresourceRange()
				.setAspectMask(vk::ImageAspectFlagBits::eDepth)
				.setBaseArrayLayer(0)
				.setLayerCount(1)
				.setBaseMipLevel(0)
				.setLevelCount(1));

		return vkCtx.getDevice().createImageView(createInfo);
	}
public:
	Renderer(const VulkanContext& vkCtx, SDL_Window* window)
		: _vkCtx(vkCtx),
		_surface(vkCtx.createSurfaceFromWindow(window)),
		_swapchain(vkCtx, _surface, 800, 600),
		_depthImage(createDepthImage(vkCtx, _swapchain)),
		_depthImageView(createDepthImageView(vkCtx, _depthImage)),
		_pipeline(vkCtx, _swapchain, _depthImageView),
		_drawCommand(vkCtx, _swapchain.getImageViews().size()),
		_transferHandler(vkCtx)
	{
		vk::SemaphoreCreateInfo semaphoreInfo;
		auto fenceInfo = vk::FenceCreateInfo()
			.setFlags(vk::FenceCreateFlagBits::eSignaled);

		_imageAvailableSemaphores.resize(maxFramesInFlight);
		_renderFinishedSemaphores.resize(maxFramesInFlight);
		_inFlightFences.resize(maxFramesInFlight);

		for (int i = 0; i < _imageAvailableSemaphores.size(); i++) {
			_imageAvailableSemaphores[i] = vkCtx.getDevice().createSemaphoreUnique(semaphoreInfo);
			_renderFinishedSemaphores[i] = vkCtx.getDevice().createSemaphoreUnique(semaphoreInfo);
			_inFlightFences[i] = vkCtx.getDevice().createFenceUnique(fenceInfo);
		}

		const std::vector<Vertex> vertices = {
			{{0.0f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
			{{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		const std::vector<uint16_t> indices = { 0, 1, 2 };

		const std::vector<Vertex> vertices2 = {
			{{0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
			{{0.5f, -0.3f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, -0.3f, 0.0f}, {0.0f, 0.0f, 1.0f}}
		};
		Model model;
		model.vertices = vertices;
		model.indices = indices;
		std::vector<Model> models = { model, {vertices2, indices} };
		_bakedModels.resize(models.size());
		submitModelBake(vkCtx, _transferHandler, models, std::span<BakedModel>(_bakedModels.data(), _bakedModels.size()));

		_drawCommand.recordCommandBuffer(_pipeline, _bakedModels);
	}

	Renderer(const Renderer&) = delete;
	void drawFrame();
	~Renderer();
};
