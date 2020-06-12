#define WIN32_LEAN_AND_MEAN

#include "gamestate.h"
#include "renderer.h"

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL_vulkan.h>

#include <atomic>
#include <iostream>
#include <chrono>
#include <thread>

using namespace std::literals::chrono_literals;

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
		Renderer renderer(vkCtx, window);

		gameState.loadFromFile(renderer, "scenes/scene.json");

		for (Object& object : gameState.objects) {
			for (DynamicObjectState& instance : object.instances) {
				physics.addObject(instance);
			}
		}
		
		std::array<GraphicsGameState, 2> gameStates;

		std::atomic<int> selectedGamestate = 0;
		gameState.initGraphicsGameState(gameStates[0]);
		gameState.initGraphicsGameState(gameStates[1]);
		gameState.updateGraphicsGameState(gameStates[0]);
		gameState.updateGraphicsGameState(gameStates[1]);

		std::chrono::time_point last = std::chrono::high_resolution_clock::now();
		
		std::mutex mutex;
		bool running = true;

		std::thread thread([&] {
			while (true) {
				renderer.drawFrame(gameStates[selectedGamestate], mutex);
			}
		});
		thread.detach();
		while (running) {
			std::chrono::duration<float> delta = std::chrono::high_resolution_clock::now() - last;
			last = std::chrono::high_resolution_clock::now();
			SDL_Event evt;
			while (SDL_PollEvent(&evt)) {
				if (evt.type == SDL_QUIT) {
					running = false;
				}
				if (evt.type == SDL_WINDOWEVENT) {
					if (evt.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
						SDL_SetWindowGrab(window, SDL_TRUE);
						SDL_SetRelativeMouseMode(SDL_TRUE);
					}
				}
				if (evt.type == SDL_KEYDOWN) {
					if (evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
						SDL_SetWindowGrab(window, SDL_FALSE);
						SDL_SetRelativeMouseMode(SDL_FALSE);
					}
				}
				if (evt.type == SDL_MOUSEMOTION) {
					gameState.cameraX -= evt.motion.xrel/1000.0;
					gameState.cameraY -= evt.motion.yrel/1000.0;
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
			gameState.cameraPos += glm::vec3(gameState.getCameraMatrix() * glm::vec4(cameraChange * delta.count() * 2.0f, 0.0f));
			mutex.lock();
			gameState.updateGraphicsGameState(gameStates[selectedGamestate]);
			selectedGamestate = !selectedGamestate;
			mutex.unlock();
			physics.stepPhysics(delta.count());
			std::this_thread::sleep_for(50ms);
		}
		
		vkCtx.getDevice().waitIdle();
	}
	catch (std::exception& e) {
		std::cout << e.what();
	}
	return 0;
}
