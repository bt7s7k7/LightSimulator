#include "pch.h"
#include "update.h"

void update(const space_t& space) {
	sdlhelp::unique_window_ptr window;
	window.reset(sdlhelp::handleSDLError(SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 500, SDL_WINDOW_RESIZABLE)));

	spdlog::info("Window initialized");

	auto surface = sdlhelp::handleSDLError(SDL_GetWindowSurface(window.get()));

	spdlog::info("Event loop starting");

	while (true) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_WINDOWEVENT) {
				if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
					goto eventLoopExit;
				} else if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
					surface = sdlhelp::handleSDLError(SDL_GetWindowSurface(window.get()));
				}
			}
		}

		SDL_Point mousePos;
		auto mouseState = SDL_GetMouseState(&mousePos.x, &mousePos.y);

		SDL_FillRect(surface, nullptr, 0);

		space.drawDebug(surface, (mouseState & SDL_BUTTON_LMASK) > 0, mousePos);

		SDL_UpdateWindowSurface(window.get());
	}
eventLoopExit:
	spdlog::info("Event loop ended");
}
