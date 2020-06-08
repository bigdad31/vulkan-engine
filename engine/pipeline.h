#pragma once

#include "vulkancontext.h"
#include "defaultuniform.h"
#include "swapchain.h"
#include "shader.h"

class Pipeline
{
	const VulkanContext& _vkCtx;
	const vk::Rect2D _scissors;
	const vk::Pipeline _pipeline;
	const vk::PipelineLayout _pipelineLayout;
	const vk::RenderPass _renderPass;
	const std::vector<vk::Framebuffer> _framebuffers;

	static Pipeline createPipeline(const VulkanContext &vkCtx, const Swapchain &swapchain, vk::ImageView depthView, DefaultUniform &uniform);
public:
	Pipeline(const VulkanContext& vkCtx, vk::Rect2D _scissors, vk::Pipeline pipeline, vk::PipelineLayout pipelineLayout, vk::RenderPass renderPass, std::vector<vk::Framebuffer> framebuffers);
	Pipeline(const VulkanContext& vkCtx, const Swapchain& swapchain, vk::ImageView depthView, DefaultUniform &uniform);
	Pipeline(Pipeline&) = delete;
	~Pipeline();

	const std::vector<vk::Framebuffer>& getFramebuffers() const {
		return _framebuffers;
	}

	vk::Rect2D getScissors() const {
		return _scissors;
	}

	vk::Pipeline getPipeline() const {
		return _pipeline;
	}
	
	vk::RenderPass getRenderPass() const {
		return _renderPass;
	}

	vk::PipelineLayout getLayout() const {
		return _pipelineLayout;
	}
};
