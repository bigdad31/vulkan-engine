#pragma once

#include "vulkancontext.h"

#include <glm/glm.hpp>

template<class T, size_t minPadding = 0>
struct Uniform {
	Buffer<T, minPadding> buffer;
	vk::DescriptorSet descriptor;
	void* data;
	Uniform() = default;
	Uniform(const VulkanContext& vkCtx, size_t size, vk::DescriptorPool pool, vk::DescriptorSetLayout layout) :
		buffer(vkCtx, size, vk::BufferUsageFlagBits::eUniformBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU) {
		auto sceneDescriptorAllocInfo = vk::DescriptorSetAllocateInfo()
			.setDescriptorPool(pool)
			.setDescriptorSetCount(1)
			.setPSetLayouts(&layout);

		vkCtx.getDevice().allocateDescriptorSets(&sceneDescriptorAllocInfo, &descriptor);
		vmaMapMemory(vkCtx.getAllocator(), buffer.allocation, &data);
	}

	static size_t getMinPadding() {
		return minPadding;
	}
};


struct SceneUniform {
	glm::mat4 proj;
	glm::mat4 view;
};


struct ModelUniform {
	glm::mat4 trans;
	glm::mat4 modelTrans;
};


class DefaultUniformLayout
{
	const VulkanContext& _vkCtx;
	vk::DescriptorSetLayout _sceneLayout;
	vk::DescriptorSetLayout _modelLayout;

	std::vector<Uniform<SceneUniform>> _sceneUniforms;
	std::vector<Uniform<ModelUniform, 256>> _modelUniforms;

	vk::DescriptorPool _descriptorPool;
public:
	DefaultUniformLayout(const VulkanContext& vkCtx, int count);

	vk::DescriptorSetLayout getSceneLayout();
	vk::DescriptorSetLayout getModelLayout();

	size_t getSceneUniformSize() const;
	size_t getModelUniformSize() const;
	std::vector<Uniform<SceneUniform>>& getSceneUniforms();
	std::vector<Uniform<ModelUniform, 256>>& getModelUniforms();
	~DefaultUniformLayout();
};

