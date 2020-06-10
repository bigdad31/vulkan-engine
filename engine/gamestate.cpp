#include "gamestate.h"
#include "rapidjson/document.h"
#include "renderer.h"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

glm::mat4 GameState::getCameraMatrix() const
{
    glm::mat4 a = glm::rotate(glm::mat4(1.0f), cameraX, { 0.0f, 0.0f, 1.0f });
    glm::mat4 b = glm::rotate(a, cameraY, { 1.0f, 0.0f, 0.0f });
    glm::mat4 c = glm::translate(glm::mat4(1.0f), cameraPos);
    
    return c*b;
}

void GameState::destroy(const VulkanContext& vkCtx)
{
    for (auto& object : objects) {
        object.model.destroy(vkCtx);
    }
}

void GameState::loadFromFile(Renderer& renderer, std::string fileName)
{
    rapidjson::Document doc;

    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    doc.Parse(buffer.data(), buffer.size());

    std::vector<Model> models;
    models.reserve(doc["models"].GetArray().Size());
    objects.reserve(doc["models"].GetArray().Size());
    for (const auto& jsonModel : doc["models"].GetArray()) {
        std::string fileName = jsonModel["file"].GetString();
        models.push_back(Model::loadFromFile("models/" + fileName));
        Object object;
        object.mass = jsonModel["mass"].GetFloat();
        const auto& collider = jsonModel["collision_shape"];
        std::string type = collider["type"].GetString();
        if (type == "box") {
            float halfX = collider["half_x"].GetFloat();
            float halfY = collider["half_y"].GetFloat();
            float halfZ = collider["half_z"].GetFloat();
            object.shape = new btBoxShape({halfX, halfY, halfZ});
        }
        else if (type == "sphere") {
            float radius = collider["radius"].GetFloat();
            object.shape = new btSphereShape(radius);
        }
        objects.push_back(object);
    }
    std::vector<BakedModel> bakedModels;
    bakedModels.resize(models.size());

    renderer.bakeModels(models, std::span<BakedModel>(bakedModels.data(), bakedModels.size()));
    for (int i = 0; i < models.size(); i++) {
        objects[i].model = bakedModels[i];
    }

    for (const auto& instance : doc["scene"]["instances"].GetArray()) {
        Object& object = objects[instance["model"].GetInt()];
        const auto& jsonPos = instance["pos"].GetArray();
        btVector3 pos = { jsonPos[0].GetFloat(), jsonPos[1].GetFloat(), jsonPos[2].GetFloat() };

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(pos);

        btVector3 localInertia(0, 0, 0);

        if (object.mass != 0) {
            object.shape->calculateLocalInertia(object.mass, localInertia);
        }

        btDefaultMotionState* motionState = new btDefaultMotionState(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(object.mass, motionState, object.shape, localInertia);
        btRigidBody* body = new btRigidBody(rbInfo);

        if (instance.HasMember("vel")) {
            const auto& jsonVel = instance["vel"].GetArray();
            btVector3 vel = { jsonVel[0].GetFloat(), jsonVel[1].GetFloat(), jsonVel[2].GetFloat() };

            body->setLinearVelocity(vel);
        }
        
        DynamicObjectState state;
        state.inertia = localInertia;
        state.motion = motionState;
        state.rigidBody = body;

        object.instances.push_back(state);
    }

    const auto& jsonCamPos = doc["scene"]["camera"]["pos"].GetArray();
    cameraPos = { jsonCamPos[0].GetFloat(), jsonCamPos[1].GetFloat(), jsonCamPos[2].GetFloat() };
}

Physics::Physics() :
    _configurator(std::make_unique<btDefaultCollisionConfiguration>()),
    _dispatcher(std::make_unique<btCollisionDispatcher>(_configurator.get())),
    _overlappingPairCache(std::make_unique<btDbvtBroadphase>()),
    _solver(std::make_unique<btSequentialImpulseConstraintSolver>()),
    _dynamicsWorld(std::make_unique<btDiscreteDynamicsWorld>(_dispatcher.get(), _overlappingPairCache.get(), _solver.get(), _configurator.get()))
{
    _dynamicsWorld->setGravity({ 0, 0, -9.81 });
}

void Physics::addObject(DynamicObjectState& state)
{
    _dynamicsWorld->addRigidBody(state.rigidBody);
}

void Physics::stepPhysics(float dt)
{
    _dynamicsWorld->stepSimulation(dt);
}
