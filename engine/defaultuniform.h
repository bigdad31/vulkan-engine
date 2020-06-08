#pragma once
#include "vulkancontext.h"
#include "glm/glm.hpp"


class DefaultUniform
{
	const VulkanContext& _vkCtx;
	vk::DescriptorSetLayout _sceneLayout;
	vk::DescriptorSetLayout _modelLayout;

	std::vector<vk::Buffer> _sceneBuffers;
	std::vector<VmaAllocation> _sceneAllocations;

	std::vector<vk::Buffer> _modelBuffers;
	std::vector<VmaAllocation> _modelAllocations;

	std::vector<vk::DescriptorSet> _sceneDescriptors;
	std::vector<vk::DescriptorSet> _modelDescriptors;

	std::vector<void*> _sceneDatas;
	std::vector<void*> _modelDatas;

	vk::DescriptorPool _descriptorPool;
public:
	DefaultUniform(const VulkanContext& vkCtx, int count);
	std::vector<vk::Buffer>& getSceneBuffers();
	std::vector<vk::Buffer>& getModelBuffers();

	std::vector<void*>& getSceneDatas();
	std::vector<void*>& getModelDatas();

	std::vector<vk::DescriptorSet>& getSceneDescriptors();
	std::vector<vk::DescriptorSet>& getModelDescriptors();

	vk::DescriptorSetLayout getSceneLayout();
	vk::DescriptorSetLayout getModelLayout();

	std::vector<VmaAllocation>& getSceneAllocations();
	std::vector<VmaAllocation>& getModelAllocations();
	~DefaultUniform();
};


struct SceneUniform {
	glm::mat4 view;
	glm::mat4 proj;
};

struct ModelUniform {
	glm::mat4 trans;
};