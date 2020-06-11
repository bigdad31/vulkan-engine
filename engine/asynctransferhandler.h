#pragma once

#include "vulkancontext.h"


class AsyncTransferHandler
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
	AsyncTransferHandler(const VulkanContext& vkCtx);

	size_t getPos() const;
	size_t getSize() const;

	bool canFit(size_t size) const;
	void beginTransferCommand();
	void addTransfer(const void* data, size_t size, vk::Buffer dstBuffer);
	void resetAndSubmitPool();

	void* mapStagingBuffer();
	void unmapStagingBuffer();
};
