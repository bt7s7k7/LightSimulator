#pragma once
#include "pch.h"
#include "vectors.h"

struct shape_t {
	color_t reflectivity;
	virtual vec2_t getNormal(const vec2_t& point) const = 0;
	virtual extent_t getDist(const vec2_t& point) const = 0;
};