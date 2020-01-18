#include "pch.h"
#include "surfaceShapes.h"

using namespace shapes;

void shapes::square(SDL_Surface* surface, SDL_Rect rect, SDL_Color color, bool fill) {
	auto [r, g, b, a] = color;
	if (fill) {
		SDL_FillRect(surface, &rect, SDL_MapRGBA(surface->format, r, g, b, a));
	} else {
		auto [x, y, w, h] = rect;
		SDL_Rect rects[] = {
			{ x, y, w, 1 },
			{ x, y, 1, h },
			{ x, y + h - 1, w, 1 },
			{ x + w - 1, y, 1, h }
		};
		SDL_FillRects(surface, rects, 4, SDL_MapRGBA(surface->format, r, g, b, a));
	}
}

void shapes::circle(SDL_Surface* surface, SDL_Point pos, int radius, SDL_Color color, bool fill) {
	auto [r, g, b, a] = color;
	auto [cx, cy] = pos;
	auto colorCode = SDL_MapRGB(surface->format, r, g, b);

	int r2 = radius * radius;
	int x = 0, x2 = 0, dx2 = 1;
	int y = radius, y2 = y * y, dy2 = 2 * y - 1;
	int sum = r2;

	// Allocate space for rects
	SDL_Rect rects[8];
	int count;

	if (!fill) {
		count = 8;
	} else {
		count = 4;
	}

	auto rectsPtr = rects;

	while (x <= y) {
		if (!fill) {
			// Render a point for each octet
			rectsPtr[0] = { cx + x, cy + y, 1, 1 };
			rectsPtr[1] = { cx + x, cy - y, 1, 1 };
			rectsPtr[2] = { cx - x, cy + y, 1, 1 };
			rectsPtr[3] = { cx - x, cy - y, 1, 1 };
			rectsPtr[4] = { cx + y, cy + x, 1, 1 };
			rectsPtr[5] = { cx + y, cy - x, 1, 1 };
			rectsPtr[6] = { cx - y, cy + x, 1, 1 };
			rectsPtr[7] = { cx - y, cy - x, 1, 1 };
		} else {
			// Draw lines for a full circle
			rectsPtr[0] = { cx - x, cy - y, cx + x - (cx - x), 1 };
			rectsPtr[1] = { cx - y, cy + x, cx + y - (cx - y), 1 };
			rectsPtr[2] = { cx - y, cy - x, cx + y - (cx - y), 1 };
			rectsPtr[3] = { cx - x, cy + y, cx + x - (cx - x), 1 };
		}
		// Draw points / lines
		SDL_FillRects(surface, rectsPtr, count, colorCode);

		sum -= dx2;
		x2 += dx2;
		x++;
		dx2 += 2;
		if (sum <= y2) {
			y--; y2 -= dy2; dy2 -= 2;
		}
	}
}

void shapes::line(SDL_Surface* surface, SDL_Point pos1, SDL_Point pos2, SDL_Color color) {
	auto [r, g, b, a] = color;
	auto [x1, y1] = pos1;
	auto [x2, y2] = pos2;
	// Getting the color
	auto colorCode = SDL_MapRGBA(surface->format, r, g, b, a);

	int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
	dx = x2 - x1;
	dy = y2 - y1;
	dx1 = std::abs(dx);
	dy1 = std::abs(dy);
	px = 2 * dy1 - dx1;
	py = 2 * dx1 - dy1;
	if (dy1 <= dx1) {
		if (dx >= 0) {
			x = x1;
			y = y1;
			xe = x2;
		} else {
			x = x2;
			y = y2;
			xe = x1;
		}
		SDL_Rect rect{ x,y,1,1 };
		SDL_FillRect(surface, &rect, colorCode);
		for (i = 0; x < xe; i++) {
			x = x + 1;
			if (px < 0) {
				px = px + 2 * dy1;
			} else {
				if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
					y = y + 1;
				} else {
					y = y - 1;
				}
				px = px + 2 * (dy1 - dx1);
			}
			SDL_Rect rect{ x,y,1,1 };
			SDL_FillRect(surface, &rect, colorCode);
		}
	} else {
		if (dy >= 0) {
			x = x1;
			y = y1;
			ye = y2;
		} else {
			x = x2;
			y = y2;
			ye = y1;
		}
		SDL_Rect rect{ x,y,1,1 };
		SDL_FillRect(surface, &rect, colorCode);
		for (i = 0; y < ye; i++) {
			y = y + 1;
			if (py <= 0) {
				py = py + 2 * dx1;
			} else {
				if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
					x = x + 1;
				} else {
					x = x - 1;
				}
				py = py + 2 * (dx1 - dy1);
			}
			SDL_Rect rect{ x,y,1,1 };
			SDL_FillRect(surface, &rect, colorCode);
		}
	}
}

void shapes::lineFast(SDL_Surface* surface, SDL_Point pos1, SDL_Point pos2, SDL_Color color) {

	auto [r, g, b, a] = color;
	auto [x1, y1] = pos1;
	auto [x2, y2] = pos2;
	// Getting the color
	auto colorCode = SDL_MapRGBA(surface->format, r, g, b, a);

	// Make sure the coordinates change the right direction
	bool flipH = false, flipV = false;
	decltype(x1) swapVal;
	if (x1 > x2) {
		swapVal = x1;
		x1 = x2;
		x2 = swapVal;
		flipH = true;
	}
	if (y1 > y2) {
		swapVal = y1;
		y1 = y2;
		y2 = swapVal;
		flipV = true;
	}

	/* If the main direction is vertical */
	bool lerpVertical = (x2 - x1) > (y2 - y1);
	// If we should flip coordinates
	/* Flip other */
	bool flipOther = false;
	/* Flip main direction */
	bool flip = false;
	// Testing for conditions
	if (lerpVertical && flipH && !flipV) flipOther = true;
	if (!lerpVertical && flipH && !flipV) flip = true;
	if (lerpVertical && !flipH && flipV) flipOther = true;
	if (!lerpVertical && !flipH && flipV) flip = true;
	/* Size of the line in the main direction */
	int dist = (lerpVertical ? y2 - y1 : x2 - x1) + 1;
	/* Size of the line in the other direction */
	int otherDist = (!lerpVertical ? y2 - y1 : x2 - x1) + 1;

	for (int i = 0; i < dist; i++) {
		int start, end;
		if (flipOther) { // Should we flip the other direction
			start = static_cast<int>((1 - ((double)i + 1) / dist) * otherDist);
			end = static_cast<int>((1 - (i + 0) / (double)dist) * otherDist);
		} else {
			start = static_cast<int>(((i + 0) / (double)dist) * otherDist);
			end = static_cast<int>((((double)i + 1) / (double)dist) * otherDist);
		}

		int ai;
		if (flip) ai = dist - i; // Should we flip the main direction
		else ai = i;

		// Rendering line segments
		SDL_Rect rect;
		if (lerpVertical) rect = { start + x1, ai + y1, end - start, 1 };
		else rect = { ai + x1, start + y1, 1, end - start };
		SDL_FillRect(surface, &rect, colorCode);
	}
}

void shapes::text(SDL_Surface* surface, SDL_Point pos, int height, textAlign align, const char* text, SDL_Color color) {
	static const std::vector<std::vector<uint8_t>> font = {
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{},
		{69,102,86},
		{69,137},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{69},
		{133,90},
		{5,82},
		{72,133,102,86},
		{72,133,102,86},
		{150},
		{25},
		{102},
		{130},
		{128,2,42,168,130},
		{20,70,42},
		{20,73,146,42},
		{4,73,149,150,98},
		{100,65,25},
		{128,1,81,90,162},
		{128,2,162,154,145},
		{8,130},
		{128,2,42,168,145},
		{128,1,25,138,162},
		{68,102},
		{72,133,102,86},
		{133,90,65,22},
		{72,133,102,86},
		{5,82,73,150},
		{72,133,102,86},
		{72,133,102,86},
		{20,73,25,18,154},
		{2,9,42,25,154},
		{132,65,22,106},
		{2,4,73,150,98},
		{128,2,42,81},
		{2,128,81},
		{128,2,42,169,149},
		{2,138,145},
		{70},
		{138,166,101},
		{2,24,161},
		{2,42},
		{32,5,88,138},
		{32,10,168},
		{97,20,73,150},
		{2,4,65},
		{65,22,105,148,90},
		{2,4,65,26},
		{132,65,21,90,162},
		{128,70},
		{2,42,168},
		{1,22,105,152},
		{2,37,90,168},
		{10,130},
		{101,80,88},
		{8,130,42},
		{132,70,106},
		{160},
		{4,70,98},
		{72,133,102,86},
		{42},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86},
		{72,133,102,86}
	};
	static const auto fontLength = font.size();

	auto [x, y] = pos;

	auto len = std::strlen(text);

	if (align == textAlign::center) {
		x -= static_cast<int>(len)* height / 2 / 2;
	} else if (align == textAlign::right) {
		x -= static_cast<int>(len)* height / 2;
	}
	for (std::size_t i = 0; i < len; i++) {
		auto c = static_cast<std::size_t>(text[i]);
		if (c >= fontLength) {
			c = fontLength - 1;
		}

		auto& graph = font[c];
		SDL_Point lPos = { x + static_cast<int>(i)* height / 2, y };

		for (auto& data : graph) {
			int x1 = (data & 0b11000000) >> 6;
			int y1 = (data & 0b00110000) >> 4;
			int x2 = (data & 0b00001100) >> 2;
			int y2 = (data & 0b00000011);

			x1 = x1 * height / 3 / 2;
			y1 = y1 * height / 3;
			x2 = x2 * height / 3 / 2;
			y2 = y2 * height / 3;

			if (x1 == x2 && y1 == y2) {
				int frac = height / 6;
				line(surface, { lPos.x + x1, lPos.y + y1 + frac }, { lPos.x + x2 + frac, lPos.y + y2 }, color);
				line(surface, { lPos.x + x1, lPos.y + y1 - frac }, { lPos.x + x2 + frac, lPos.y + y2 }, color);
				line(surface, { lPos.x + x1, lPos.y + y1 - frac }, { lPos.x + x2 - frac, lPos.y + y2 }, color);
				line(surface, { lPos.x + x1, lPos.y + y1 + frac }, { lPos.x + x2 - frac, lPos.y + y2 }, color);
			} else line(surface, { lPos.x + x1, lPos.y + y1 }, { lPos.x + x2, lPos.y + y2 }, color);
		}
	}
}
