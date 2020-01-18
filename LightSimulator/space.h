#pragma once
#include "vectors.h"
#include "line.h"
#include "pch.h"

struct space_t {
	vec2_t size;
	std::vector<line_t> lines;

	void drawDebug(SDL_Surface* surface, bool drawMouse, const SDL_Point& mousePos) const;

	extent_t getMinDist(const vec2_t& point) const;

	void loadFromFile(const std::filesystem::path& file);

	inline void clear() {
		size = vec2_t(0, 0);
		lines.clear();
	}

	inline space_t() : size(0, 0), lines() {};
};