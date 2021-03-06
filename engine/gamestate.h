#pragma once

#include "model.h"

#include <bullet/btBulletDynamicsCommon.h>

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include <chrono>
#include <memory>
#include <vector>


class Renderer;
class VulkanContext;

struct DynamicObjectState {
	btDefaultMotionState* motion;
	btRigidBody* rigidBody;
	btVector3 inertia;
};

struct Object {
	BakedModel model;
	btScalar mass;
	btCollisionShape* shape;
	std::vector<DynamicObjectState> instances;
};

struct DynamicGraphicsInstanceState {
	glm::mat4 globalTransform;
	glm::vec3 velocity;
	glm::vec3 rotVelocity;
};

struct GraphicsObjectState {
	BakedModel model;
	std::vector<DynamicGraphicsInstanceState> instances;
};

struct GraphicsGameState {
	std::vector<GraphicsObjectState> objects;
	glm::mat4 sceneMatrix;
	std::chrono::high_resolution_clock::time_point timeStamp;
};


struct GameState
{
	std::vector<Object> objects;
	float cameraX = 0;
	float cameraY = 0;
	glm::vec3 cameraPos{ 0.0f, -3.0f, 4.0f };

	glm::mat4 getCameraMatrix() const;

	void destroy(const VulkanContext& vkCtx);
	void loadFromFile(Renderer& renderer, std::string fileName);
	void initGraphicsGameState(GraphicsGameState& gameState);
	void updateGraphicsGameState(GraphicsGameState& gameState);
};

class Physics {
	std::unique_ptr<btDefaultCollisionConfiguration> _configurator;
	std::unique_ptr<btCollisionDispatcher> _dispatcher;
	std::unique_ptr<btBroadphaseInterface> _overlappingPairCache;
	std::unique_ptr<btSequentialImpulseConstraintSolver> _solver;
	std::unique_ptr<btDynamicsWorld> _dynamicsWorld;
public:
	Physics();
	void addObject(DynamicObjectState& state);
	void stepPhysics(float dt);
};