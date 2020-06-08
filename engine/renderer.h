#pragma once

#include "pipeline.h"
#include "depthstencil.h"
#include "asynctransferhandler.h"
#include "model.h"
#include "defaultuniform.h"
#include "gamestate.h"

constexpr int maxFramesInFlight = 2;

class Renderer
{
	const VulkanContext& _vkCtx;
	vk::SurfaceKHR _surface;
	Swapchain _swapchain;
	DepthStencil _depthStencil;
	DefaultUniformLayout _uniform;
	Pipeline _pipeline;
	AsyncTransferHandler _transferHandler;

	std::vector<vk::UniqueHandle<vk::Semaphore, vk::DispatchLoaderStatic>> _imageAvailableSemaphores;
	std::vector<vk::UniqueHandle<vk::Semaphore, vk::DispatchLoaderStatic>> _renderFinishedSemaphores;
	std::vector<vk::UniqueHandle<vk::Fence, vk::DispatchLoaderStatic>> _inFlightFences;

	vk::CommandPool _commandPool;
	std::vector<vk::CommandBuffer> _commandBuffers;
	size_t _frame = 0;

public:
	Renderer(const VulkanContext& vkCtx, SDL_Window* window)
		: _vkCtx(vkCtx),
		_surface(vkCtx.createSurfaceFromWindow(window)),
		_swapchain(vkCtx, _surface, 800, 600),
		_depthStencil(vkCtx, 800, 600),
		_uniform(vkCtx, _swapchain.getImageViews().size()),
		_pipeline(vkCtx, _swapchain, _depthStencil.getImageView(), _uniform),
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

		uint32_t graphicsQueue = vkCtx.getQueueFamilies().graphicsInd.value();
		_commandPool = vkCtx.getDevice().createCommandPool(vk::CommandPoolCreateInfo().setQueueFamilyIndex(graphicsQueue).setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer));

		auto allocInfo = vk::CommandBufferAllocateInfo()
			.setCommandBufferCount(_swapchain.getImageViews().size())
			.setCommandPool(_commandPool)
			.setLevel(vk::CommandBufferLevel::ePrimary);

		_commandBuffers = vkCtx.getDevice().allocateCommandBuffers(allocInfo);

		int count = _swapchain.getImageViews().size();
	}

	void bakeModels(const std::span<Model>& models, std::span<BakedModel>& bakedModels) {
		submitModelBake(_vkCtx, _transferHandler, models, bakedModels);
	}

	Renderer(const Renderer&) = delete;
	void drawFrame(const GameState& gameState);
	~Renderer();
};
