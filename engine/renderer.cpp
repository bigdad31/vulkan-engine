#include "renderer.h"

#include "gamestate.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <array>

Renderer::Renderer(const VulkanContext& vkCtx, SDL_Window* window) : _vkCtx(vkCtx),
_surface(vkCtx.createSurfaceFromWindow(window)),
_swapchain(vkCtx, _surface, 800, 600),
_depthStencil(vkCtx, 800, 600),
_uniform(vkCtx, _swapchain.getImageCount()),
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
		.setCommandBufferCount(_swapchain.getImageCount())
		.setCommandPool(_commandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary);

	_commandBuffers = vkCtx.getDevice().allocateCommandBuffers(allocInfo);
}

void Renderer::bakeModels(const std::vector<Model>& models, std::vector<BakedModel>& bakedModels)
{
	submitModelBake(_vkCtx, _transferHandler, models, bakedModels);
}

void Renderer::drawFrame(const GameState& gameState)
{
	_vkCtx.getDevice().waitForFences({ _inFlightFences[_frame].get() }, true, UINT64_MAX);

	_vkCtx.getDevice().resetFences({ _inFlightFences[_frame].get() });
	vk::Queue queue = _vkCtx.getGraphicsQueue(0);
	uint32_t imageIndex = _swapchain.acquireNextImage(_imageAvailableSemaphores[_frame].get());

	const vk::CommandBuffer& commandBuffer = _commandBuffers[imageIndex];
	auto clearColor = vk::ClearValue().setColor(vk::ClearColorValue().setFloat32({ 0, 0, 0, 1.0 }));
	auto clearDepth = vk::ClearValue().setColor(vk::ClearColorValue().setFloat32({ 1.0, 0, 0, 0 }));

	vk::ClearValue clearValues[] = { clearColor, clearDepth };

	glm::mat4 coordTransform = {
		{1, 0, 0, 0},
		{0, 0, -1, 0},
		{0, -1, 0, 0},
		{0, 0, 0, 1}
	};

	glm::mat4 sceneMatrix = glm::perspective(glm::radians(90.0f), 4.0f / 3.0f, 0.1f, 100.0f) * coordTransform * glm::inverse(gameState.getCameraMatrix());

	unsigned i = 0;
	for (const auto& object : gameState.objects) {
		for (const auto& instance : object.instances) {
			btTransform transform;
			instance.motion->getWorldTransform(transform);
			glm::mat4 trans;
			transform.getOpenGLMatrix((btScalar*)&trans);

			ModelUniform uniform;
			uniform.modelTrans = trans;
			uniform.trans = sceneMatrix * trans;
			memcpy((char*)(_uniform.getModelUniforms()[imageIndex].data) + i * _uniform.getModelUniformSize(), &uniform, sizeof(ModelUniform));
			i++;
		}
	}
	vmaFlushAllocation(_vkCtx.getAllocator(), _uniform.getModelUniforms()[imageIndex].buffer.allocation, 0, VK_WHOLE_SIZE);
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
	i = 0;
	for (const auto& object : gameState.objects) {
		vk::DeviceSize offset{};
		commandBuffer.bindVertexBuffers(0, 1, &object.model.vertices.data, &offset);
		commandBuffer.bindIndexBuffer(object.model.indices.data, 0, vk::IndexType::eUint16);
		for (const auto& model : object.instances) {
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipeline.getLayout(), 1, { _uniform.getModelUniforms()[imageIndex].descriptor }, { i * (unsigned)_uniform.getModelUniformSize() });
			commandBuffer.drawIndexed((uint32_t)object.model.indices.size, 1, 0, 0, 0);
			i++;
		}
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

	queue.submit({ submitInfo }, _inFlightFences[_frame].get());

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

}
