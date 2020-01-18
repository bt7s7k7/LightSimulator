#include "pch.h"
#undef main
#include "update.h"
#include "space.h"

int main() try {
	sdlhelp::handleSDLError(SDL_Init(SDL_INIT_VIDEO));
	spdlog::info("SDL initialized");

	space_t space;

	update(space);
	
	return 0;
} catch (const std::exception & err) {
	spdlog::critical(err.what());
	return 1;
}