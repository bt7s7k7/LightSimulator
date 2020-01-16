#include "pch.h"
#include "update.h"

void update() {
	spdlog::info("Event loop starting");

	while (true) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_WINDOWEVENT) {
				if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
					goto eventLoopExit;
				}
			}
		}
	}
	eventLoopExit:
	spdlog::info("Event loop ended");
}
