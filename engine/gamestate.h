#pragma once

#include "model.h"
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

struct DynamicObjectState {
	glm::vec3 position;
	glm::vec3 velocity;
	glm::quat rotation;
	glm::quat rotationalVelocity;
};

struct DynamicObjectProperties {
	float mass;
};

struct Object {
	BakedModel model;
	DynamicObjectProperties properties;
	std::vector<DynamicObjectState> instances;
};

struct GameState
{
	std::vector<Object> objects;
	glm::mat4 camera;
	const std::vector<Object>& getObjects() const;
	glm::mat4 getCamera() const;

	void destroy(const VulkanContext& vkCtx);
};
