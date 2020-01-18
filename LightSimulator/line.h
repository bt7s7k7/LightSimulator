#pragma once
#include "vectors.h"

struct line_t {
	vec2_t a;
	vec2_t b;

	inline extent_t getDist(const vec2_t& p) const {
		// Source https://iquilezles.org/www/articles/distfunctions/distfunctions.htm
		vec2_t pa = p - a, ba = b - a;
		extent_t h = (dot(pa, ba) / dot(ba, ba));

		if (h < 0) h = 0;
		if (h > 1) h = 1;

		return (pa - ba * h).length();
	}

	inline line_t(vec2_t a, vec2_t b) : a(a), b(b) {};
};