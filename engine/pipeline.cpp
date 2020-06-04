#include "pipeline.h"

#include <algorithm>
#include "model.h"

Pipeline Pipeline::createPipeline(const VulkanContext& vkCtx, const Swapchain& swapchain)
{
	Shader vertShader = Shader::loadShaderFromFile(vkCtx, "shaders/default.vert.spv");
	Shader fragShader = Shader::loadShaderFromFile(vkCtx, "shaders/default.frag.spv");

	auto vertStageInfo = vk::PipelineShaderStageCreateInfo()
		.setModule(vertShader.getShader())
		.setStage(vk::ShaderStageFlagBits::eVertex)
		.setPName("main");

	auto fragStageInfo = vk::PipelineShaderStageCreateInfo()
		.setModule(fragShader.getShader())
		.setStage(vk::ShaderStageFlagBits::eFragment)
		.setPName("main");

	vk::PipelineShaderStageCreateInfo shaderStages[] = { vertStageInfo, fragStageInfo };

	auto vertexInputBinding = Vertex::getVertexDescription();
	auto vertexInputAttributes = Vertex::getAttributeDescriptions();

	auto vertexState = vk::PipelineVertexInputStateCreateInfo()
		.setVertexAttributeDescriptionCount(vertexInputAttributes.size())
		.setPVertexAttributeDescriptions(vertexInputAttributes.data())
		.setVertexBindingDescriptionCount(1)
		.setPVertexBindingDescriptions(&vertexInputBinding);
	
	auto vertexAssembly = vk::PipelineInputAssemblyStateCreateInfo()
		.setPrimitiveRestartEnable(false)
		.setTopology(vk::PrimitiveTopology::eTriangleList);
	auto viewport = swapchain.getViewport();
	auto scissors = swapchain.getScissors();

	auto viewportState = vk::PipelineViewportStateCreateInfo()
		.setScissorCount(1)
		.setPScissors(&scissors)
		.setViewportCount(1)
		.setPViewports(&viewport);

	auto rasterizer = vk::PipelineRasterizationStateCreateInfo()
		.setDepthClampEnable(false)
		.setCullMode(vk::CullModeFlagBits::eBack)
		.setRasterizerDiscardEnable(false)
		.setFrontFace(vk::FrontFace::eClockwise)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setLineWidth(1)
		.setDepthBiasEnable(false);
	
	auto multiSampling = vk::PipelineMultisampleStateCreateInfo()
		.setSampleShadingEnable(false)
		.setRasterizationSamples(vk::SampleCountFlagBits::e1)
		.setMinSampleShading(1.0f)
		.setPSampleMask(nullptr)
		.setAlphaToCoverageEnable(false)
		.setAlphaToOneEnable(false);

	auto colorBlendAttachment = vk::PipelineColorBlendAttachmentState()
		.setBlendEnable(false)
		.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

	auto colorBlendState = vk::PipelineColorBlendStateCreateInfo()
		.setAttachmentCount(1)
		.setPAttachments(&colorBlendAttachment)
		.setLogicOpEnable(false);

	auto pipelineInfo = vk::PipelineLayoutCreateInfo();
	vk::PipelineLayout pipelineLayout = vkCtx.getDevice().createPipelineLayout(pipelineInfo);

	auto colorAttachment = vk::AttachmentDescription()
		.setFormat(swapchain.getFormat().format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	auto colorAttachmentRef = vk::AttachmentReference()
		.setAttachment(0)
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	auto subpass = vk::SubpassDescription()
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorAttachmentRef);

	auto subpassDependencies = vk::SubpassDependency()
		.setSrcSubpass(VK_SUBPASS_EXTERNAL)
		.setDstSubpass(0)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

	auto renderPassInfo = vk::RenderPassCreateInfo()
		.setAttachmentCount(1)
		.setPAttachments(&colorAttachment)
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(1)
		.setPDependencies(&subpassDependencies);

	vk::RenderPass renderPass = vkCtx.getDevice().createRenderPass(renderPassInfo);
	auto graphicsPipelineInfo = vk::GraphicsPipelineCreateInfo()
		.setStageCount(2)
		.setPStages(shaderStages)
		.setPVertexInputState(&vertexState)
		.setPInputAssemblyState(&vertexAssembly)
		.setPColorBlendState(&colorBlendState)
		.setPViewportState(&viewportState)
		.setPMultisampleState(&multiSampling)
		.setPRasterizationState(&rasterizer)
		.setLayout(pipelineLayout)
		.setRenderPass(renderPass)
		.setSubpass(0);
	vk::Pipeline pipeline = vkCtx.getDevice().createGraphicsPipeline(vk::PipelineCache(), graphicsPipelineInfo);

	std::vector<vk::Framebuffer> framebuffers (swapchain.getImageViews().size());

	std::transform(swapchain.getImageViews().begin(), swapchain.getImageViews().end(), framebuffers.begin(), [&](const vk::ImageView &imageView) {
		auto frameBufferInfo = vk::FramebufferCreateInfo()
			.setAttachmentCount(1)
			.setPAttachments(&imageView)
			.setRenderPass(renderPass)
			.setWidth(swapchain.getWidth())
			.setHeight(swapchain.getHeight())
			.setLayers(1);
		return vkCtx.getDevice().createFramebuffer(frameBufferInfo);
		});

	return Pipeline(vkCtx, scissors, pipeline, pipelineLayout, renderPass, framebuffers);
}

Pipeline::Pipeline(const VulkanContext& vkCtx, vk::Rect2D scissors, vk::Pipeline pipeline, vk::PipelineLayout pipelineLayout, vk::RenderPass renderPass, std::vector<vk::Framebuffer> framebuffers)
	: _vkCtx(vkCtx),
	_scissors(scissors),
	_pipeline(pipeline),
	_pipelineLayout(pipelineLayout),
	_renderPass(renderPass),
	_framebuffers(framebuffers)
{
}

Pipeline::Pipeline(const VulkanContext& vkCtx, const Swapchain& swapchain)
	: Pipeline(createPipeline(vkCtx, swapchain))
{
}

Pipeline::~Pipeline()
{
	for (auto framebuffer : _framebuffers) {
		_vkCtx.deviceDestroy(framebuffer);
	}
	_vkCtx.deviceDestroy(_pipeline);
	_vkCtx.deviceDestroy(_pipelineLayout);
	_vkCtx.deviceDestroy(_renderPass);
}
