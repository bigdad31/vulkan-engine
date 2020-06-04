#pragma once

#include "vulkancontext.h"
#include "pipeline.h"
#include "model.h"


class DrawCommand
{
	const VulkanContext& _vkCtx;
	vk::CommandPool _commandPool;
	std::vector<vk::CommandBuffer> _commandBuffers;
	static DrawCommand createDrawCommand(const VulkanContext &vkCtx, uint32_t nCommandBuffers);
public:
	DrawCommand(const VulkanContext& vkCtx, uint32_t nCommandBuffers);
	DrawCommand(const VulkanContext& vkCtx, vk::CommandPool commandPool, std::vector<vk::CommandBuffer> commandBuffers);
	DrawCommand(DrawCommand&&) = default;
	~DrawCommand();
	void recordCommandBuffer(const Pipeline& pipeline, const std::vector<BakedModel> &models) const;
	void recordCommandBuffer(const Pipeline& pipeline, const std::vector<BakedModel> &models, int ind) const;
	vk::CommandBuffer getCommandBuffer(uint32_t ind) const {
		return _commandBuffers[ind];
	}
	void reset() const;
};
