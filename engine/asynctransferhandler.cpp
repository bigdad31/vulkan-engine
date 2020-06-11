#include "asynctransferhandler.h"

AsyncTransferHandler::AsyncTransferHandler(const VulkanContext& vkCtx) : _vkCtx(vkCtx), _pos(0), _size(16777216)
{
	auto stagingInfo = vk::BufferCreateInfo()
		.setUsage(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setSize(_size);

	_stagingBuffer = vkCtx.getDevice().createBuffer(stagingInfo);
	auto memoryRequirements = vkCtx.getDevice().getBufferMemoryRequirements(_stagingBuffer);

	auto stagingAllocInfo = vk::MemoryAllocateInfo()
		.setAllocationSize(memoryRequirements.size)
		.setMemoryTypeIndex(vkCtx.findMemoryType(memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible));

	_stagingMemory = vkCtx.getDevice().allocateMemory(stagingAllocInfo);
	vkCtx.getDevice().bindBufferMemory(_stagingBuffer, _stagingMemory, 0);

	_commandPool = vkCtx.getDevice().createCommandPool(vk::CommandPoolCreateInfo().setQueueFamilyIndex(vkCtx.getQueueFamilies().transferInd.value()).setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer));

	auto allocInfo = vk::CommandBufferAllocateInfo()
		.setCommandBufferCount(1)
		.setCommandPool(_commandPool)
		.setLevel(vk::CommandBufferLevel::ePrimary);
	_commandBuffer = _vkCtx.getDevice().allocateCommandBuffers(allocInfo)[0];
}

size_t AsyncTransferHandler::getPos() const
{
	return _pos;
}

size_t AsyncTransferHandler::getSize() const
{
	return _size;
}

bool AsyncTransferHandler::canFit(size_t size) const
{
	return _pos + size < _size;
}

void AsyncTransferHandler::beginTransferCommand()
{
	auto beginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	_commandBuffer.begin(beginInfo);
	_data = (uint8_t*)mapStagingBuffer();
}

void AsyncTransferHandler::addTransfer(const void* data, size_t size, vk::Buffer dstBuffer)
{
	memcpy(_data + _pos, data, size);

	_commandBuffer.copyBuffer(_stagingBuffer, dstBuffer, { vk::BufferCopy()
		.setSrcOffset(_pos)
		.setSize(size) });

	_pos += size;
}

void AsyncTransferHandler::resetAndSubmitPool()
{
	_pos = 0;

	_commandBuffer.end();
	unmapStagingBuffer();
	auto submitInfo = vk::SubmitInfo()
		.setCommandBufferCount(1)
		.setPCommandBuffers(&_commandBuffer);

	_vkCtx.getTransferQueue(0).submit({ submitInfo }, vk::Fence());

	_vkCtx.getTransferQueue(0).waitIdle();
	_commandBuffer.reset(vk::CommandBufferResetFlagBits());
}

void* AsyncTransferHandler::mapStagingBuffer()
{
	return _vkCtx.getDevice().mapMemory(_stagingMemory, 0, _size);
}

void AsyncTransferHandler::unmapStagingBuffer()
{
	_vkCtx.getDevice().unmapMemory(_stagingMemory);
}
