#include "model.h"

vk::VertexInputBindingDescription Vertex::getVertexDescription()
{
	return vk::VertexInputBindingDescription()
		.setBinding(0)
		.setStride(sizeof(Vertex))
		.setInputRate(vk::VertexInputRate::eVertex);
}

std::array<vk::VertexInputAttributeDescription, 2> Vertex::getAttributeDescriptions()
{
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
			.setOffset(offsetof(Vertex, normal))
	};
}

Model Model::loadFromFile(std::string fileName)
{
	Model model;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(fileName, aiProcess_Triangulate);
	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		const aiMesh* mesh = scene->mMeshes[i];
		for (unsigned k = 0; k < mesh->mNumVertices; k++) {
			const aiVector3D vertex = mesh->mVertices[k];
			const aiVector3D normal = mesh->mNormals[k];
			Vertex vert;
			vert.pos = { vertex.x, vertex.y, vertex.z };
			vert.normal = { normal.x, normal.y, normal.z };
			model.vertices.push_back(vert);
		}
		for (unsigned k = 0; k < mesh->mNumFaces; k++) {
			model.indices.push_back(mesh->mFaces[k].mIndices[0]);
			model.indices.push_back(mesh->mFaces[k].mIndices[1]);
			model.indices.push_back(mesh->mFaces[k].mIndices[2]);
		}
	}

	return model;
}

void BakedModel::destroy(const VulkanContext& vkCtx)
{
	vertices.destroy(vkCtx);
	indices.destroy(vkCtx);
}

void submitModelBake(const VulkanContext& vkCtx, AsyncTransferHandler& transferHandler, const std::vector<Model>& models, std::vector<BakedModel>& bakedModels)
{
	for (int i = 0; i < bakedModels.size(); i++) {
		bakedModels[i].vertices = Buffer<Vertex>(vkCtx, models[i].vertices.size(), vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);
		bakedModels[i].indices = Buffer<uint16_t>(vkCtx, models[i].indices.size(), vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst);
	}

	transferHandler.beginTransferCommand();
	for (int i = 0; i < bakedModels.size(); i++) {
		const auto& model = models[i];
		auto& bakedModel = bakedModels[i];

		if (!transferHandler.canFit(sizeof(Vertex) * model.vertices.size() + sizeof(uint16_t) * model.indices.size())) {
			transferHandler.resetAndSubmitPool();
			transferHandler.beginTransferCommand();
		}
		transferHandler.addTransfer(model.vertices.data(), sizeof(Vertex) * model.vertices.size(), bakedModel.vertices.data);
		transferHandler.addTransfer(model.indices.data(), sizeof(uint16_t) * model.indices.size(), bakedModel.indices.data);
	}
	transferHandler.resetAndSubmitPool();
}