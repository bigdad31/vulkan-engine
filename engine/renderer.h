#pragma once

#include "asynctransferhandler.h"
#include "defaultuniform.h"
#include "depthstencil.h"
#include "model.h"
#include "pipeline.h"

#include <mutex>

struct GraphicsGameState;

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
	Renderer(const VulkanContext& vkCtx, SDL_Window* window);

	void bakeModels(const std::vector<Model>& models, std::vector<BakedModel>& bakedModels);

	Renderer(const Renderer&) = delete;
	void drawFrame(GraphicsGameState& gameState, std::mutex& mutex);
	~Renderer();
};
