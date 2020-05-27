#include "shader.h"
#include <fstream>

Shader Shader::loadShaderFromFile(const VulkanContext& vkCtx, std::string path)
{
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	auto shaderInfo = vk::ShaderModuleCreateInfo()
		.setCodeSize(buffer.size())
		.setPCode(reinterpret_cast<const uint32_t*>(buffer.data()));
	
	vk::ShaderModule shader = vkCtx.getDevice().createShaderModule(shaderInfo);

	return Shader(vkCtx, shader);
}

Shader::Shader(const VulkanContext& vkCtx, std::string path)
	: Shader(loadShaderFromFile(_vkCtx, path))
{
}

Shader::Shader(const VulkanContext& vkCtx, vk::ShaderModule shader) :
	_vkCtx(vkCtx),
	_shader(shader)
{
}

vk::ShaderModule Shader::getShader() const
{
	return _shader;
}

Shader::~Shader()
{
	_vkCtx.getDevice().destroy(_shader);
}
