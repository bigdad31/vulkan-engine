// vulkan.cpp : Defines the entry point for the application.
//

#include "engine.h"

#include <vulkan/vulkan.hpp>

#include <SDL2/SDL.h>
#include <SDL_vulkan.h>
#include <iostream>

#include <optional>
#include <vector>

#include "renderer.h"
#include <chrono>

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
		Renderer renderer(vkCtx, window);
		{
			bool running = true;
			while (running) {
				SDL_Event evt;
				while (SDL_PollEvent(&evt)) {
					if (evt.type == SDL_QUIT) {
						running = false;
					}
				}
				
				renderer.drawFrame();
			}
		}
		vkCtx.getDevice().waitIdle();
	}
	catch (std::exception &e) {
		std::cout << e.what();
	}
	return 0;
}
