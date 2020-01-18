#pragma once
#include "pch.h"

namespace shapes {
	void square(SDL_Surface* surface, SDL_Rect rect, SDL_Color color, bool fill);
	void circle(SDL_Surface* surface, SDL_Point pos, int radius, SDL_Color color, bool fill);
	void line(SDL_Surface* surface, SDL_Point a, SDL_Point b, SDL_Color color);
	/* Drawing of this line is faster, but it's uglier and can miss the target by one pixel */
	void lineFast(SDL_Surface* surface, SDL_Point a, SDL_Point b, SDL_Color color);

	enum class textAlign {
		left,
		center,
		right
	};

	void text(SDL_Surface* surface, SDL_Point pos, int height, textAlign align, const char * text, SDL_Color color);

}