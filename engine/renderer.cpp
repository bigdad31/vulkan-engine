#include "renderer.h"
#include <array>

void Renderer::drawFrame()
{
	_vkCtx.getDevice().waitForFences({ _inFlightFences[_frame].get() }, true, UINT64_MAX);


	_vkCtx.getDevice().resetFences({ _inFlightFences[_frame].get() });
	vk::Queue queue = _vkCtx.getGraphicsQueue(0);
	uint32_t imageIndex = _swapchain.acquireNextImage(_imageAvailableSemaphores[_frame].get());
	_drawCommand.recordCommandBuffer(_pipeline, _bakedModels, imageIndex);
	vk::Semaphore waitSemaphores[] = { _imageAvailableSemaphores[_frame].get() };
	vk::Semaphore signalSemaphores[] = { _renderFinishedSemaphores[_frame].get() };
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	vk::CommandBuffer drawCommandBuffer = _drawCommand.getCommandBuffer(imageIndex);
	auto submitInfo = vk::SubmitInfo()
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(waitSemaphores)
		.setPWaitDstStageMask(waitStages)
		.setCommandBufferCount(1)
		.setPCommandBuffers(&drawCommandBuffer)
		.setSignalSemaphoreCount(1)
		.setPSignalSemaphores(signalSemaphores);

	queue.submit({submitInfo}, _inFlightFences[_frame].get());

	vk::SwapchainKHR swapchains[] = { _swapchain.getSwapchain() };

	auto presentInfo = vk::PresentInfoKHR()
		.setSwapchainCount(1)
		.setPSwapchains(swapchains)
		.setPImageIndices(&imageIndex)
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(signalSemaphores);
	
	queue.presentKHR(&presentInfo);
	
	_frame = (_frame + 1) % maxFramesInFlight;
}

Renderer::~Renderer()
{
	for (auto &bakedModel : _bakedModels) {
		bakedModel.destroy(_vkCtx);
	}
	_vkCtx.destroy(_surface);
}
