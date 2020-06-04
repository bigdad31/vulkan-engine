#pragma once

#include <glm/glm.hpp>
#include <span>
#include "vulkancontext.h"
#include "transferhandler.h"


struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;

	static vk::VertexInputBindingDescription getVertexDescription() {
		return vk::VertexInputBindingDescription()
			.setBinding(0)
			.setStride(sizeof(Vertex))
			.setInputRate(vk::VertexInputRate::eVertex);
	}


	static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
		return {
			vk::VertexInputAttributeDescription()
				.setBinding(0)
				.setLocation(0)
				.setFormat(vk::Format::eR32G32B32Sfloat)
				.setOffset(offsetof(Vertex, pos)),
			vk::VertexInputAttributeDescription()
				.setBinding(0)
				.setLocation(1)
				.setFormat(vk::Format::eR32G32B32Sfloat)
				.setOffset(offsetof(Vertex, color))
		};
	}
};

struct Model
{
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
};

struct BakedModel {
	vk::Buffer vertexData;
	size_t nVertices;
	vk::Buffer indexData;
	size_t nIndices;
	VmaAllocation vertexAllocation;
	VmaAllocationInfo vertexAllocInfo;
	VmaAllocation indexAllocation;
	VmaAllocationInfo indexAllocInfo;

	void destroy(const VulkanContext& vkCtx) {
		vmaFreeMemory(vkCtx.getAllocator(), vertexAllocation);
		vmaFreeMemory(vkCtx.getAllocator(), indexAllocation);
		vkCtx.deviceDestroy(vertexData);
		vkCtx.deviceDestroy(indexData);
	}
};

inline void submitModelBake(const VulkanContext& vkCtx, TransferHandler& transferHandler, const std::span<Model>& models, std::span<BakedModel>& bakedModels) {
	for (int i = 0; i < bakedModels.size(); i++) {
		const auto& model = models[i];
		auto& bakedModel = bakedModels[i];

		auto vertexInfo = vk::BufferCreateInfo()
			.setUsage(vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setSize(sizeof(Vertex) * model.vertices.size());


		auto indexInfo = vk::BufferCreateInfo()
			.setUsage(vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst)
			.setSharingMode(vk::SharingMode::eExclusive)
			.setSize(sizeof(uint32_t) * model.indices.size());

		bakedModel.nVertices = model.vertices.size();
		bakedModel.nIndices = model.indices.size();
		
		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateBuffer(vkCtx.getAllocator(), (VkBufferCreateInfo*)(&indexInfo), &allocCreateInfo, (VkBuffer*)&bakedModel.indexData, &bakedModel.indexAllocation, &bakedModel.indexAllocInfo);
		vmaCreateBuffer(vkCtx.getAllocator(), (VkBufferCreateInfo*)(&vertexInfo), &allocCreateInfo, (VkBuffer*)&bakedModel.vertexData, &bakedModel.vertexAllocation, &bakedModel.vertexAllocInfo);
	}

	transferHandler.beginTransferCommand();
	for (int i = 0; i < bakedModels.size(); i++) {
		const auto& model = models[i];
		auto& bakedModel = bakedModels[i];

		if (!transferHandler.canFit(sizeof(Vertex) * model.vertices.size() + sizeof(uint16_t) * model.indices.size())) {
			transferHandler.resetAndSubmitPool();
			transferHandler.beginTransferCommand();
		}
		transferHandler.addTransfer(model.vertices.data(), sizeof(Vertex) * model.vertices.size(), bakedModel.vertexData);
		transferHandler.addTransfer(model.indices.data(), sizeof(uint16_t) * model.indices.size(), bakedModel.indexData);
	}
	transferHandler.resetAndSubmitPool();
}