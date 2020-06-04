#pragma once
#include "vulkancontext.h"

class TransferHandler
{
	const VulkanContext& _vkCtx;
	vk::CommandPool _commandPool;
	vk::CommandBuffer _commandBuffer;
	vk::Buffer _stagingBuffer;
	vk::DeviceMemory _stagingMemory;
	uint8_t* _data;

	size_t _pos;
	size_t _size;
public:
	TransferHandler(const VulkanContext& vkCtx) : _vkCtx(vkCtx), _pos(0), _size(16777216) {
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

	size_t getPos() const {
		return _pos;
	}

	size_t getSize() const {
		return _size;
	}

	bool canFit(size_t size) const {
		return _pos + size < _size;
	}

	void beginTransferCommand() {
		auto beginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		_commandBuffer.begin(beginInfo);
		_data = (uint8_t*)mapStagingBuffer();
	}

	void addTransfer(const void* data, size_t size, vk::Buffer dstBuffer) {
		memcpy(_data + _pos, data, size);

		_commandBuffer.copyBuffer(_stagingBuffer, dstBuffer, { vk::BufferCopy()
			.setSrcOffset(_pos)
			.setSize(size) });
		
		_pos += size;
	}

	void resetAndSubmitPool() {
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

	void* mapStagingBuffer() {
		return _vkCtx.getDevice().mapMemory(_stagingMemory, 0, _size);
	}

	void unmapStagingBuffer() {
		_vkCtx.getDevice().unmapMemory(_stagingMemory);
	}
};
