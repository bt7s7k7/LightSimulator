#pragma once
#include "pch.h"
#include "vectors.h"

struct shape_t {
	virtual vec2_t getNormal(const vec2_t& point) const = 0;
};