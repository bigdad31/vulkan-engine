#include "defaultuniform.h"

DefaultUniform::DefaultUniform(const VulkanContext& vkCtx, int count) : _vkCtx(vkCtx)
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
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex);

	auto modelLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(1)
		.setPBindings(&sceneLayoutBinding);

	_modelLayout = _vkCtx.getDevice().createDescriptorSetLayout(modelLayoutCreateInfo);

	vk::DescriptorPoolSize descriptorPoolSize[] = {
		vk::DescriptorPoolSize()
			.setDescriptorCount(2*count)
			.setType(vk::DescriptorType::eUniformBuffer)
	};

	auto descriptorPoolInfo = vk::DescriptorPoolCreateInfo()
		.setMaxSets(2*count)
		.setPPoolSizes(descriptorPoolSize)
		.setPoolSizeCount(1);

	_descriptorPool = vkCtx.getDevice().createDescriptorPool(descriptorPoolInfo);

	std::vector<vk::DescriptorSetLayout> sceneLayouts(count, _sceneLayout);
	auto sceneDescriptorAllocInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(_descriptorPool)
		.setDescriptorSetCount(sceneLayouts.size())
		.setPSetLayouts(sceneLayouts.data());

	_sceneDescriptors = vkCtx.getDevice().allocateDescriptorSets(sceneDescriptorAllocInfo);

	std::vector<vk::DescriptorSetLayout> modelLayouts(count, _modelLayout);
	auto modelDescriptorAllocInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(_descriptorPool)
		.setDescriptorSetCount(modelLayouts.size())
		.setPSetLayouts(modelLayouts.data());

	_modelDescriptors = vkCtx.getDevice().allocateDescriptorSets(modelDescriptorAllocInfo);
	_modelBuffers.resize(count);
	_sceneBuffers.resize(count);
	_modelAllocations.resize(count);
	_sceneAllocations.resize(count);
	_modelDatas.resize(count);
	_sceneDatas.resize(count);

	for (int i = 0; i < count; i++) {
		auto modelBufferInfo = vk::BufferCreateInfo()
			.setSharingMode(vk::SharingMode::eExclusive)
			.setSize(65536)
			.setUsage(vk::BufferUsageFlagBits::eUniformBuffer);
		_modelBuffers[i] = _vkCtx.getDevice().createBuffer(modelBufferInfo);

		VmaAllocationCreateInfo	modelAllocInfo{};
		modelAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		vmaAllocateMemoryForBuffer(_vkCtx.getAllocator(), _modelBuffers[i], &modelAllocInfo, _modelAllocations.data() + i, nullptr);
		vmaBindBufferMemory(_vkCtx.getAllocator(), _modelAllocations[i], _modelBuffers[i]);
		vmaMapMemory(_vkCtx.getAllocator(), _modelAllocations[i], &_modelDatas[i]);

		auto modelDescBufferInfo = vk::DescriptorBufferInfo()
			.setBuffer(_modelBuffers[i])
			.setOffset(0)
			.setRange(vk::DeviceSize(sizeof(ModelUniform)));

		auto modelWriteInfo = vk::WriteDescriptorSet()
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDstSet(_modelDescriptors[i])
			.setPBufferInfo(&modelDescBufferInfo);

		auto sceneBufferInfo = vk::BufferCreateInfo()
			.setSharingMode(vk::SharingMode::eExclusive)
			.setSize(sizeof(SceneUniform))
			.setUsage(vk::BufferUsageFlagBits::eUniformBuffer);
		_sceneBuffers[i] = _vkCtx.getDevice().createBuffer(sceneBufferInfo);

		VmaAllocationCreateInfo	sceneAllocInfo{};
		sceneAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		VmaAllocationInfo info;
		vmaAllocateMemoryForBuffer(_vkCtx.getAllocator(), _sceneBuffers[i], &modelAllocInfo, _sceneAllocations.data() + i, &info);
		vmaBindBufferMemory(_vkCtx.getAllocator(), _sceneAllocations[i], _sceneBuffers[i]);
		vmaMapMemory(_vkCtx.getAllocator(), _sceneAllocations[i], &_sceneDatas[i]);


		auto sceneDescBufferInfo = vk::DescriptorBufferInfo()
			.setBuffer(_sceneBuffers[i])
			.setOffset(0)
			.setRange(vk::DeviceSize(sizeof(SceneUniform)));

		auto sceneWriteInfo = vk::WriteDescriptorSet()
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDstSet(_sceneDescriptors[i])
			.setPBufferInfo(&sceneDescBufferInfo);


		vkCtx.getDevice().updateDescriptorSets({ sceneWriteInfo, modelWriteInfo }, {});
	}
}

std::vector<vk::Buffer>& DefaultUniform::getSceneBuffers()
{
	return _sceneBuffers;
}

std::vector<vk::Buffer>& DefaultUniform::getModelBuffers()
{
	return _modelBuffers;
}

std::vector<void*>& DefaultUniform::getSceneDatas()
{
	return _sceneDatas;
}

std::vector<void*>& DefaultUniform::getModelDatas()
{
	return _modelDatas;
}

std::vector<vk::DescriptorSet>& DefaultUniform::getSceneDescriptors()
{
	return _sceneDescriptors;
}

std::vector<vk::DescriptorSet>& DefaultUniform::getModelDescriptors()
{
	return _modelDescriptors;
}

vk::DescriptorSetLayout DefaultUniform::getSceneLayout()
{
	return _sceneLayout;
}

vk::DescriptorSetLayout DefaultUniform::getModelLayout()
{
	return _modelLayout;
}

std::vector<VmaAllocation>& DefaultUniform::getSceneAllocations()
{
	return _sceneAllocations;
}

std::vector<VmaAllocation>& DefaultUniform::getModelAllocations()
{
	return _modelAllocations;
}

DefaultUniform::~DefaultUniform()
{
	for (int i = 0; i < _modelBuffers.size(); i++) {
		vmaUnmapMemory(_vkCtx.getAllocator(), _modelAllocations[i]);
		vmaUnmapMemory(_vkCtx.getAllocator(), _sceneAllocations[i]);
		vmaFreeMemory(_vkCtx.getAllocator(), _modelAllocations[i]);
		vmaFreeMemory(_vkCtx.getAllocator(), _sceneAllocations[i]);
	}
	_vkCtx.deviceDestroy(_sceneLayout);
	_vkCtx.deviceDestroy(_modelLayout);
	_vkCtx.deviceDestroy(_descriptorPool);
}
