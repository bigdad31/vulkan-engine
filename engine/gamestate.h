#pragma once

#include "model.h"
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include <memory>

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

struct GameState
{
	std::vector<Object> objects;
	glm::mat4 camera;
	const std::vector<Object>& getObjects() const;
	glm::mat4 getCamera() const;

	void destroy(const VulkanContext& vkCtx);
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