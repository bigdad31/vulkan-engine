#pragma once

#include "vulkancontext.h"

class Shader
{
	const VulkanContext& _vkCtx;
	vk::ShaderModule _shader;
public:
	Shader(const VulkanContext& vkCtx, std::string path);
	Shader(const VulkanContext& vkCtx, vk::ShaderModule shader);

	static Shader loadShaderFromFile(const VulkanContext& vkCtx, std::string path);

	vk::ShaderModule getShader() const;

	~Shader();
};
