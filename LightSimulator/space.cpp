#include "pch.h"
#include "space.h"

void space_t::drawDebug(SDL_Surface* surface, bool drawMouse, const SDL_Point& mousePos) const {
	extent_t w = surface->w;
	extent_t h = surface->h;

	extent_t zoom = 1;
	if ((size.x * zoom) < w) {
		zoom = w / size.x;
	}

	if ((size.y * zoom) < h) {
		zoom = h / size.y;
	}

	if ((size.x * zoom) > w) {
		zoom = w / size.x;
	}

	if ((size.y * zoom) > h) {
		zoom = h / size.y;
	}

	SDL_Rect targetSpace = {
		0, 0,
		(int)std::floor(size.x * zoom),
		(int)std::floor(size.y * zoom)
	};

	targetSpace.x = surface->w / 2 - targetSpace.w / 2;
	targetSpace.y = surface->h / 2 - targetSpace.h / 2;

	shapes::square(surface, SDL_Rect{ targetSpace.x - 1, targetSpace.y - 1, targetSpace.w + 2, targetSpace.h + 2 }, SDL_Color{ 255, 255, 255, 255 }, false);

	auto localToScreen = [&](const vec2_t& point) -> SDL_Point {
		return ((point * zoom) + vec2_t(targetSpace.x, targetSpace.y));
	};

	auto screenToLocal = [&](const SDL_Point& point) -> vec2_t {
		return (vec2_t(point) - vec2_t(targetSpace.x, targetSpace.y)) * (1 / zoom);
	};
 
	for (auto& line : lines) {
		shapes::line(surface, localToScreen(line.a), localToScreen(line.b), SDL_Color{ 0,255,0,255 });
	}

	if (drawMouse && !lines.empty()) {
		shapes::circle(surface, mousePos, (int)std::floor(getMinDist(screenToLocal(mousePos)) * zoom), SDL_Color{ 0,255,255,255 }, false);
	}
}

extent_t space_t::getMinDist(const vec2_t& point) const {
	extent_t min = std::numeric_limits<extent_t>::infinity();
	for (size_t i = 0, len = lines.size(); i < len; i++) {
		extent_t dist = lines[i].getDist(point);
		if (dist < min) {
			min = dist;
		}
	}
	return min;
}
