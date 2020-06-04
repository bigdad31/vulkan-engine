#define WIN32_LEAN_AND_MEAN
#include <vulkan/vulkan.hpp>

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL_vulkan.h>
#include <iostream>

#include <optional>
#include <vector>
#include "model.h"
#include "transferhandler.h"

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
