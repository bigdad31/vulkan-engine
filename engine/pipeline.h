#pragma once

#include "defaultuniform.h"
#include "shader.h"
#include "swapchain.h"
#include "vulkancontext.h"


class Pipeline
{
	const VulkanContext& _vkCtx;
	const vk::Rect2D _scissors;
	const vk::Pipeline _pipeline;
	const vk::PipelineLayout _pipelineLayout;
	const vk::RenderPass _renderPass;
	const std::vector<vk::Framebuffer> _framebuffers;

	static Pipeline createPipeline(const VulkanContext& vkCtx, const Swapchain& swapchain, vk::ImageView depthView, DefaultUniformLayout& uniform);
public:
	Pipeline(const VulkanContext& vkCtx, vk::Rect2D _scissors, vk::Pipeline pipeline, vk::PipelineLayout pipelineLayout, vk::RenderPass renderPass, std::vector<vk::Framebuffer> framebuffers);
	Pipeline(const VulkanContext& vkCtx, const Swapchain& swapchain, vk::ImageView depthView, DefaultUniformLayout& uniform);
	Pipeline(Pipeline&) = delete;
	~Pipeline();

	const std::vector<vk::Framebuffer>& getFramebuffers() const;
	vk::Rect2D getScissors() const;

	vk::Pipeline getPipeline() const;

	vk::RenderPass getRenderPass() const;
	vk::PipelineLayout getLayout() const;
};
