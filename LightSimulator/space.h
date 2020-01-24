#pragma once
#include "vectors.h"
#include "line.h"
#include "pch.h"

template <typename T>
struct objectHolder_t {
	std::vector<T> items;

	extent_t getMinDist(const vec2_t& point) const {
		extent_t min = std::numeric_limits<extent_t>::infinity();
		for (size_t i = 0, len = items.size(); i < len; i++) {
			extent_t dist = items[i].getDist(point);
			if (dist < min) {
				min = dist;
			}
		}
		return min;
	}

	std::pair<const T*, extent_t> getClosest(const vec2_t& point) const {
		extent_t min = std::numeric_limits<extent_t>::infinity();
		const T* target = nullptr;
		for (size_t i = 0, len = items.size(); i < len; i++) {
			extent_t dist = items[i].getDist(point);
			if (dist < min) {
				min = dist;
				target = &items[i];
			}
		}
		return { target, min };
	}

	objectHolder_t() : items() {}
};

struct space_t : public objectHolder_t<line_t> {
	vec2_t size;

	void drawDebug(SDL_Surface* surface, bool drawMouse, const SDL_Point& mousePos) const;

	extent_t getGlobalMinDist(const vec2_t& point) const;
	std::pair<const shape_t*, extent_t> getClosestShape(const vec2_t& point) const;

	void loadFromFile(const std::filesystem::path& file);

	inline void clear() {
		size = vec2_t(0, 0);
		objectHolder_t<line_t>::items.clear();
	}

	inline space_t() : size(0, 0) {};
};