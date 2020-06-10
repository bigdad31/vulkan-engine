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
		VulkanContext vkCtx(window);

		GameState gameState;

		Model model = Model::loadFromFile("models/bruh.fbx");
		Model floor = Model::loadFromFile("models/ground.fbx");
		std::vector<Model> models = { model, floor};
		std::vector<BakedModel> bakedModels(models.size());
		Renderer renderer(vkCtx, window);
		renderer.bakeModels(models, std::span<BakedModel>(bakedModels.data(), bakedModels.size()));

		std::vector<std::vector<DynamicObjectState>> states = {
			{{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::quat(glm::vec3()), glm::quat()}},
			{{glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::quat(glm::vec3()), glm::quat()}}
		};
		gameState.objects.resize(bakedModels.size());
		for (int i = 0; i < models.size(); i++) {
			gameState.objects[i].model = bakedModels[i];
			gameState.objects[i].instances = { states[i] };
		}
		gameState.camera = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		SDL_SetRelativeMouseMode(SDL_TRUE);
		float cameraX = 0;
		float cameraY = 0;
		glm::vec3 cameraPos{0.0f, -3.0f, 0.0f};
		std::chrono::time_point last = std::chrono::high_resolution_clock::now();
		{
			bool running = true;
			while (running) {
				std::chrono::duration<float> delta = std::chrono::high_resolution_clock::now() - last;
				last = std::chrono::high_resolution_clock::now();
				gameState.objects[0].instances[0].position += glm::vec3(0, 0, 0) * delta.count();
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
				cameraPos += glm::vec3(a*glm::vec4(cameraChange * delta.count(), 0.0f));
				glm::mat4 b = glm::rotate(a, cameraY, { 1.0f, 0.0f, 0.0f });
				glm::mat4 c = glm::translate(glm::mat4(1.0f), cameraPos);
				gameState.camera = c*b;
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
