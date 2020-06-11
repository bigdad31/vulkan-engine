#include "defaultuniform.h"

DefaultUniformLayout::DefaultUniformLayout(const VulkanContext& vkCtx, int count) : _vkCtx(vkCtx)
{
	auto sceneLayoutBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);

	auto sceneLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(1)
		.setPBindings(&sceneLayoutBinding);

	_sceneLayout = _vkCtx.getDevice().createDescriptorSetLayout(sceneLayoutCreateInfo);
	auto modelLayoutBinding = vk::DescriptorSetLayoutBinding()
		.setBinding(0)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);

	auto modelLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(1)
		.setPBindings(&modelLayoutBinding);

	_modelLayout = _vkCtx.getDevice().createDescriptorSetLayout(modelLayoutCreateInfo);

	vk::DescriptorPoolSize descriptorPoolSize[] = {
		vk::DescriptorPoolSize()
			.setDescriptorCount(count)
			.setType(vk::DescriptorType::eUniformBufferDynamic),
		vk::DescriptorPoolSize()
			.setDescriptorCount(count)
			.setType(vk::DescriptorType::eUniformBuffer)
	};

	auto descriptorPoolInfo = vk::DescriptorPoolCreateInfo()
		.setMaxSets(2 * count)
		.setPPoolSizes(descriptorPoolSize)
		.setPoolSizeCount(1);

	_descriptorPool = vkCtx.getDevice().createDescriptorPool(descriptorPoolInfo);
	_sceneUniforms.resize(count);
	_modelUniforms.resize(count);

	for (int i = 0; i < count; i++) {
		auto& modelUniform = _modelUniforms[i];
		auto& sceneUniform = _sceneUniforms[i];

		modelUniform = Uniform<ModelUniform, 256>(vkCtx, 128, _descriptorPool, _modelLayout);
		sceneUniform = Uniform<SceneUniform>(vkCtx, 1, _descriptorPool, _sceneLayout);

		auto modelDescBufferInfo = vk::DescriptorBufferInfo()
			.setBuffer(modelUniform.buffer.data)
			.setOffset(0)
			.setRange(vk::DeviceSize(sizeof(ModelUniform)));

		auto modelWriteInfo = vk::WriteDescriptorSet()
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBufferDynamic)
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDstSet(modelUniform.descriptor)
			.setPBufferInfo(&modelDescBufferInfo);

		auto sceneDescBufferInfo = vk::DescriptorBufferInfo()
			.setBuffer(sceneUniform.buffer.data)
			.setOffset(0)
			.setRange(vk::DeviceSize(sizeof(SceneUniform)));

		auto sceneWriteInfo = vk::WriteDescriptorSet()
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDstSet(sceneUniform.descriptor)
			.setPBufferInfo(&sceneDescBufferInfo);

		vkCtx.getDevice().updateDescriptorSets({ sceneWriteInfo, modelWriteInfo }, {});
	}
}


vk::DescriptorSetLayout DefaultUniformLayout::getSceneLayout()
{
	return _sceneLayout;
}

vk::DescriptorSetLayout DefaultUniformLayout::getModelLayout()
{
	return _modelLayout;
}

size_t DefaultUniformLayout::getSceneUniformSize() const
{
	return size_t();
}

size_t DefaultUniformLayout::getModelUniformSize() const
{
	return Uniform<ModelUniform, 256>::getMinPadding();
}

std::vector<Uniform<SceneUniform>>& DefaultUniformLayout::getSceneUniforms()
{
	return _sceneUniforms;
}

std::vector<Uniform<ModelUniform, 256>>& DefaultUniformLayout::getModelUniforms()
{
	return _modelUniforms;
}

DefaultUniformLayout::~DefaultUniformLayout()
{
	for (int i = 0; i < _modelUniforms.size(); i++) {
		vmaUnmapMemory(_vkCtx.getAllocator(), _modelUniforms[i].buffer.allocation);
		vmaUnmapMemory(_vkCtx.getAllocator(), _sceneUniforms[i].buffer.allocation);
	}
	_vkCtx.deviceDestroy(_sceneLayout);
	_vkCtx.deviceDestroy(_modelLayout);
	_vkCtx.deviceDestroy(_descriptorPool);
}
