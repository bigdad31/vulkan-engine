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
