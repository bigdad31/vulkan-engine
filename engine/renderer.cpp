#include "renderer.h"
#include <array>
#include <glm/gtc/matrix_transform.hpp>

void Renderer::drawFrame()
{
	_vkCtx.getDevice().waitForFences({ _inFlightFences[_frame].get() }, true, UINT64_MAX);

	_vkCtx.getDevice().resetFences({ _inFlightFences[_frame].get() });
	vk::Queue queue = _vkCtx.getGraphicsQueue(0);
	uint32_t imageIndex = _swapchain.acquireNextImage(_imageAvailableSemaphores[_frame].get());

	const vk::CommandBuffer& commandBuffer = _commandBuffers[imageIndex];
	auto clearColor = vk::ClearValue().setColor(vk::ClearColorValue().setFloat32({ 0, 0, 0, 1.0 }));
	auto clearDepth = vk::ClearValue().setColor(vk::ClearColorValue().setFloat32({ 1.0, 0, 0, 0 }));

	vk::ClearValue clearValues[] = { clearColor, clearDepth };

	SceneUniform scene = {
		glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f),
		glm::lookAt(glm::vec3{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f})
	};

	memcpy(_uniform.getSceneUniforms()[imageIndex].data, &scene, sizeof(SceneUniform));
	//vmaFlushAllocation(_vkCtx.getAllocator(), _uniform.getModelAllocations()[imageIndex], 0, VK_WHOLE_SIZE);
	auto renderArea = vk::Rect2D();

	auto beginInfo = vk::CommandBufferBeginInfo();

	auto renderPassBeginInfo = vk::RenderPassBeginInfo()
		.setClearValueCount(2)
		.setPClearValues(clearValues)
		.setFramebuffer(_pipeline.getFramebuffers()[imageIndex])
		.setRenderArea(_pipeline.getScissors())
		.setRenderPass(_pipeline.getRenderPass());
	commandBuffer.reset({});
	commandBuffer.begin(beginInfo);
		commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
			commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline.getPipeline());
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipeline.getLayout(), 0, { _uniform.getSceneUniforms()[imageIndex].descriptor }, { });
			for (const auto& model : _bakedModels) {
				vk::DeviceSize offset{};
				commandBuffer.bindVertexBuffers(0, 1, &model.vertices.data, &offset);
				commandBuffer.bindIndexBuffer(model.indices.data, 0, vk::IndexType::eUint16);
				commandBuffer.drawIndexed(model.indices.size, 1, 0, 0, 0);
			}
		commandBuffer.endRenderPass();
	commandBuffer.end();

	vk::Semaphore waitSemaphores[] = { _imageAvailableSemaphores[_frame].get() };
	vk::Semaphore signalSemaphores[] = { _renderFinishedSemaphores[_frame].get() };
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	auto submitInfo = vk::SubmitInfo()
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(waitSemaphores)
		.setPWaitDstStageMask(waitStages)
		.setCommandBufferCount(1)
		.setPCommandBuffers(&commandBuffer)
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
}
