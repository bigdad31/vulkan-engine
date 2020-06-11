#pragma once

#include <glm/glm.hpp>
#include <span>
#include "vulkancontext.h"
#include "asynctransferhandler.h"
#include <fstream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>         
#include <assimp/postprocess.h> 

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;

	static vk::VertexInputBindingDescription getVertexDescription();


	static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions();
};

struct Model
{
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	static Model loadFromFile(std::string fileName);
};

struct BakedModel {
	Buffer<Vertex> vertices;
	Buffer<uint16_t> indices;

	void destroy(const VulkanContext& vkCtx);
};

void submitModelBake(const VulkanContext& vkCtx, AsyncTransferHandler& transferHandler, const std::vector<Model>& models, std::vector<BakedModel>& bakedModels);