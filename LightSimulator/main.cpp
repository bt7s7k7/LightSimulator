#include "pch.h"
#undef main
#include "update.h"

int main() try {
	sdlhelp::handleSDLError(SDL_Init(SDL_INIT_VIDEO));
	spdlog::info("SDL initialized");

	sdlhelp::shared_window_ptr window;
	window.reset(sdlhelp::handleSDLError(SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 100, 100, SDL_WINDOW_RESIZABLE)));

	spdlog::info("Window initialized");

	update();
	
	return 0;
} catch (const std::exception & err) {
	spdlog::critical(err.what());
	return 1;
}