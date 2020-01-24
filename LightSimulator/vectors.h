#pragma once
#include "pch.h"

using extent_t = double;

struct vec2_t {
	extent_t x;
	extent_t y;

	inline vec2_t(extent_t x_, extent_t y_) : x(x_), y(y_) {}
	inline vec2_t(const SDL_Point& point) : x(point.x), y(point.y) {}

	inline vec2_t operator +(const vec2_t& other) const {
		return vec2_t(x + other.x, y + other.y);
	}

	inline vec2_t operator -(const vec2_t& other) const {
		return vec2_t(x - other.x, y - other.y);
	}

	inline vec2_t operator *(const vec2_t& other) const {
		return vec2_t(x * other.x, y * other.y);
	}

	inline vec2_t operator *(extent_t other) const {
		return vec2_t(x * other, y * other);
	}

	inline vec2_t operator -() const {
		return vec2_t(-x, -y);
	}

	inline vec2_t perpendicular() const {
		return vec2_t(
			-y * 1,
			x * 1
		);
	}

	inline extent_t length() const {
		return std::sqrt(x * x + y * y);
	}

	inline vec2_t normalize() const {
		return *this * (1 / length());
	}

	inline operator SDL_Point() const {
		return SDL_Point{ (int)std::floor(x), (int)std::floor(y) };
	}
};

inline extent_t dot(const vec2_t& a, const vec2_t& b) {
	return a.x * b.x + a.y * b.y;
}

struct color_t {
	extent_t r;
	extent_t g;
	extent_t b;

	inline color_t(extent_t r, extent_t g, extent_t b) : r(r), g(g), b(b) {}

	inline color_t operator+(const color_t& other) {
		return color_t(
			r + other.r,
			g + other.g,
			b + other.b
		);
	}

	inline color_t operator*(const color_t& other) {
		return color_t(
			r * other.r,
			g * other.g,
			b * other.b
		);
	}

	inline color_t operator*(extent_t other) {
		return color_t(
			r * other,
			g * other,
			b * other
		);
	}

	inline operator SDL_Color() {
		return SDL_Color{
			(Uint8)std::floor(r * 255),
			(Uint8)std::floor(g * 255),
			(Uint8)std::floor(b * 255),
			255
		};
	}
};