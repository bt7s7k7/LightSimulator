#include "pch.h"
#undef main
#include "update.h"
#include "space.h"

int main() try {
	sdlhelp::handleSDLError(SDL_Init(SDL_INIT_VIDEO));
	spdlog::info("SDL initialized");

	space_t space;
	space.lines.push_back(line_t(vec2_t(25, 25), vec2_t(75, 25)));
	space.lines.push_back(line_t(vec2_t(25, 25), vec2_t(25, 75)));
	space.lines.push_back(line_t(vec2_t(75, 25), vec2_t(75, 75)));
	space.lines.push_back(line_t(vec2_t(25, 75), vec2_t(45, 75)));
	space.lines.push_back(line_t(vec2_t(55, 75), vec2_t(75, 75)));
	space.size = vec2_t(100, 100);

	update(space);
	
	return 0;
} catch (const std::exception & err) {
	spdlog::critical(err.what());
	return 1;
}