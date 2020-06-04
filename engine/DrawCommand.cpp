#include "DrawCommand.h"

DrawCommand DrawCommand::createDrawCommand(const VulkanContext& vkCtx, uint32_t nCommandBuffers)
{
	uint32_t graphicsQueue = vkCtx.getQueueFamilies().graphicsInd.value();
	auto commandPool = vkCtx.getDevice().createCommandPool(vk::CommandPoolCreateInfo().setQueueFamilyIndex(graphicsQueue).setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer));

	auto allocInfo = vk::CommandBufferAllocateInfo()
		.setCommandBufferCount(nCommandBuffers)
		.setCommandPool(commandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary);

	std::vector<vk::CommandBuffer> commandBuffers = vkCtx.getDevice().allocateCommandBuffers(allocInfo);
	return DrawCommand(vkCtx, commandPool, commandBuffers);
}

DrawCommand::DrawCommand(const VulkanContext& vkCtx, uint32_t nCommandBuffers)
	: DrawCommand(createDrawCommand(vkCtx, nCommandBuffers))
{
}

DrawCommand::DrawCommand(const VulkanContext& vkCtx, vk::CommandPool commandPool, std::vector<vk::CommandBuffer> commandBuffers)
	: _vkCtx(vkCtx),
	_commandPool(commandPool),
	_commandBuffers(commandBuffers)
{
}

DrawCommand::~DrawCommand()
{
	_vkCtx.deviceDestroy(_commandPool);
}

void DrawCommand::recordCommandBuffer(const Pipeline& pipeline, const std::vector<BakedModel>& models) const
{
	for (int i = 0; i < _commandBuffers.size(); i++) {
		recordCommandBuffer(pipeline, models, i);
	}
}

void DrawCommand::recordCommandBuffer(const Pipeline& pipeline, const std::vector<BakedModel>& models, int ind) const
{
	const vk::CommandBuffer& commandBuffer = _commandBuffers[ind];
	auto clearColor = vk::ClearValue().setColor(vk::ClearColorValue().setFloat32({ 0, 0, 0, 1.0 }));
	auto clearDepth = vk::ClearValue().setColor(vk::ClearColorValue().setFloat32({1.0, 0, 0, 0}));

	vk::ClearValue clearValues[] = { clearColor, clearDepth };

	auto renderArea = vk::Rect2D();

	auto beginInfo = vk::CommandBufferBeginInfo();

	auto renderPassBeginInfo = vk::RenderPassBeginInfo()
		.setClearValueCount(2)
		.setPClearValues(clearValues)
		.setFramebuffer(pipeline.getFramebuffers()[ind])
		.setRenderArea(pipeline.getScissors())
		.setRenderPass(pipeline.getRenderPass());
	commandBuffer.reset({});
	commandBuffer.begin(beginInfo);
	commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.getPipeline());

	for (const auto& model : models) {
		vk::DeviceSize offset{};
		commandBuffer.bindVertexBuffers(0, 1, &model.vertexData, &offset);
		commandBuffer.bindIndexBuffer(model.indexData, 0, vk::IndexType::eUint16);
		commandBuffer.drawIndexed(model.nIndices, 1, 0, 0, 0);
	}

	commandBuffer.endRenderPass();
	commandBuffer.end();
}

void DrawCommand::reset() const
{
	_vkCtx.getDevice().resetCommandPool(_commandPool, vk::CommandPoolResetFlagBits::eReleaseResources);
}
