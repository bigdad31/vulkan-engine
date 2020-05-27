#pragma once

#include "pipeline.h"
#include "DrawCommand.h"

constexpr int maxFramesInFlight = 2;

class Renderer
{
	const VulkanContext& _vkCtx;
	vk::SurfaceKHR _surface;
	Swapchain _swapchain;
	Pipeline _pipeline;
	DrawCommand _drawCommand;

	std::vector<vk::UniqueHandle<vk::Semaphore, vk::DispatchLoaderStatic>> _imageAvailableSemaphores;
	std::vector<vk::UniqueHandle<vk::Semaphore, vk::DispatchLoaderStatic>> _renderFinishedSemaphores;
	std::vector<vk::UniqueHandle<vk::Fence, vk::DispatchLoaderStatic>> _inFlightFences;
	size_t _frame = 0;

public:
	Renderer(const VulkanContext& vkCtx, SDL_Window* window)
		: _vkCtx(vkCtx),
		_surface(vkCtx.createSurfaceFromWindow(window)),
		_swapchain(vkCtx, _surface, 800, 600),
		_pipeline(vkCtx, _swapchain),
		_drawCommand(vkCtx, _swapchain.getImageViews().size())
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
		_drawCommand.recordCommandBuffer(_pipeline);
	}

	Renderer(const Renderer&) = delete;
	void drawFrame();
	~Renderer();
};
