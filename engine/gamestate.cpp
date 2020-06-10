#include "gamestate.h"

const std::vector<Object>& GameState::getObjects() const
{
    return objects;
}

glm::mat4 GameState::getCamera() const
{
    return camera;
}

void GameState::destroy(const VulkanContext& vkCtx)
{
    for (auto& object : objects) {
        object.model.destroy(vkCtx);
    }
}

Physics::Physics() :
    _configurator(std::make_unique<btDefaultCollisionConfiguration>()),
    _dispatcher(std::make_unique<btCollisionDispatcher>(_configurator.get())),
    _overlappingPairCache(std::make_unique<btDbvtBroadphase>()),
    _solver(std::make_unique<btSequentialImpulseConstraintSolver>()),
    _dynamicsWorld(std::make_unique<btDiscreteDynamicsWorld>(_dispatcher.get(), _overlappingPairCache.get(), _solver.get(), _configurator.get()))
{
    _dynamicsWorld->setGravity({ 0, -9.81, 0 });
}
