#define WIN32_LEAN_AND_MEAN
#include <vulkan/vulkan.hpp>

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL_vulkan.h>
#include <iostream>

#include <optional>
#include <vector>
#include "model.h"
#include "asynctransferhandler.h"
#include <glm/gtx/quaternion.hpp>

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

		const std::vector<Vertex> vertices = {
			{{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
		};

		const std::vector<uint16_t> indices = { 0, 1, 2 };

		const std::vector<Vertex> vertices2 = {
			{{0.0f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}
		};
		Model model;
		model.vertices = vertices;
		model.indices = indices;
		std::vector<Model> models = { model, {vertices2, indices} };
		std::vector<BakedModel> bakedModels(models.size());
		Renderer renderer(vkCtx, window);
		renderer.bakeModels(models, std::span<BakedModel>(bakedModels.data(), bakedModels.size()));

		std::vector<std::vector<DynamicObjectState>> states = {
			{{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::quat(glm::vec3()), glm::quat()}},
			{{glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::quat(glm::vec3()), glm::quat()}}
		};
		gameState.objects.resize(bakedModels.size());
		for (int i = 0; i < models.size(); i++) {
			gameState.objects[i].model = bakedModels[i];
			gameState.objects[i].instances = { states[i] };
		}
		gameState.camera = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		{
			bool running = true;
			while (running) {
				SDL_Event evt;
				while (SDL_PollEvent(&evt)) {
					if (evt.type == SDL_QUIT) {
						running = false;
					}
				}
				
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
