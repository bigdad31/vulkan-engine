#define WIN32_LEAN_AND_MEAN
#include <vulkan/vulkan.hpp>

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL_vulkan.h>
#include <iostream>

#include <optional>
#include <vector>
#include "model.h"
#include "asynctransferhandler.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "renderer.h"
#include <chrono>
#include "gamestate.h"

#undef main

#ifdef NDEBUG
constexpr bool debug = false;
#else
constexpr bool debug = true;
#endif

int main()
{
	unsigned windowFlags = SDL_WINDOW_VULKAN;
	try {
		SDL_Window* window = SDL_CreateWindow("Bruh", 500, 500, 800, 600, windowFlags);
		SDL_SetWindowBordered(window, SDL_TRUE);
		SDL_SetRelativeMouseMode(SDL_TRUE);

		VulkanContext vkCtx(window);
		GameState gameState;
		Physics physics;

		Model model = Model::loadFromFile("models/bruh.fbx");
		Model floor = Model::loadFromFile("models/ground.fbx");
		std::vector<Model> models = { model, floor, model};
		std::vector<btScalar> masses = { 1.0, 0.0, 1.0 };
		std::vector<btCollisionShape*> shapes = { new btSphereShape(1.0), new btBoxShape(btVector3(btScalar(10.), btScalar(10.0), btScalar(0.5))), new btSphereShape(1.0) };
		std::vector<btVector3> startVelocities = { {0, 0, 0.1}, {0, 0, 0}, {0, 1, 0} };
		std::vector<btVector3> startPositions = { {0, 0, 0}, {0, 0, -3}, {0, 0, 2.5} };
		std::vector<BakedModel> bakedModels(models.size());

		Renderer renderer(vkCtx, window);
		renderer.bakeModels(models, std::span<BakedModel>(bakedModels.data(), bakedModels.size()));

		gameState.objects.resize(bakedModels.size());
		for (int i = 0; i < models.size(); i++) {
			gameState.objects[i].model = bakedModels[i];

			btTransform transform;
			transform.setIdentity();
			transform.setOrigin(startPositions[i]);
			bool isDynamic = masses[i] != 0;
			btVector3 localInertia(0, 0, 0);
			if (isDynamic)
				shapes[i]->calculateLocalInertia(masses[i], localInertia);

			btDefaultMotionState* motionState = new btDefaultMotionState(transform);

			btRigidBody::btRigidBodyConstructionInfo rbInfo(masses[i], motionState, shapes[i], localInertia);
			btRigidBody* body = new btRigidBody(rbInfo);
			body->setLinearVelocity(startVelocities[i]);
			DynamicObjectState state;
			state.inertia = localInertia;
			state.motion = motionState;
			state.rigidBody = body;

			gameState.objects[i].instances = { state };
			gameState.objects[i].mass = masses[i];
			gameState.objects[i].shape = shapes[i];

			physics.addObject(state);
		}

		float cameraX = 0;
		float cameraY = 0;
		glm::vec3 cameraPos{0.0f, -3.0f, 0.0f};
		std::chrono::time_point last = std::chrono::high_resolution_clock::now();
		{
			bool running = true;
			while (running) {
				std::chrono::duration<float> delta = std::chrono::high_resolution_clock::now() - last;
				last = std::chrono::high_resolution_clock::now();
				SDL_Event evt;
				while (SDL_PollEvent(&evt)) {
					if (evt.type == SDL_QUIT) {
						running = false;
					}
					if (evt.type == SDL_MOUSEMOTION) {
						cameraX -= evt.motion.xrel * delta.count();
						cameraY -= evt.motion.yrel * delta.count();
					}
				}
				const Uint8* keystate = SDL_GetKeyboardState(nullptr);
				glm::vec3 cameraChange{};
				if (keystate[SDL_SCANCODE_A]) {
					cameraChange += glm::vec3{ -1, 0, 0 };
				}
				if (keystate[SDL_SCANCODE_D]) {
					cameraChange += glm::vec3{ 1, 0, 0 };
				}
				if (keystate[SDL_SCANCODE_S]) {
					cameraChange += glm::vec3{ 0, -1, 0 };
				}
				if (keystate[SDL_SCANCODE_W]) {
					cameraChange += glm::vec3{ 0, 1, 0 };
				}
				if (keystate[SDL_SCANCODE_SPACE]) {
					cameraChange += glm::vec3{ 0, 0, 1 };
				}
				if (keystate[SDL_SCANCODE_LCTRL]) {
					cameraChange += glm::vec3{ 0, 0, -1 };
				}

				glm::mat4 a = glm::rotate(glm::mat4(1.0f), cameraX, { 0.0f, 0.0f, 1.0f });
				glm::mat4 b = glm::rotate(a, cameraY, { 1.0f, 0.0f, 0.0f });
				cameraPos += glm::vec3(b * glm::vec4(cameraChange * delta.count() * 2.0f, 0.0f));
				glm::mat4 c = glm::translate(glm::mat4(1.0f), cameraPos);
				gameState.camera = c*b;
				physics.stepPhysics(delta.count());
				renderer.drawFrame(gameState);
			}
		}
		vkCtx.getDevice().waitIdle();
	}
	catch (std::exception &e) {
		std::cout << e.what();
	}
	return 0;
}
