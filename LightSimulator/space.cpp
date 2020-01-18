#include "pch.h"
#include "space.h"
#include "exceptions.h"

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

void space_t::loadFromFile(const std::filesystem::path& path) {
	std::ifstream file;

	file.open(path);

	if (file.fail()) {
		throw except::fileOpenFail_ex(std::filesystem::absolute(path).string(), errno);
	}

	auto json = nlohmann::json::parse(file);

	auto parseVec2 = [&](const nlohmann::json::value_type& value, const std::string& name) {
		if (!value.is_array()
			|| value.size() != 2
			|| !value.at(0).is_number()
			|| !value.at(1).is_number()
			) throw except::configValueMistyped_ex(name, "[x : number, y : number]");

		return vec2_t(value.at(0), value.at(1));
	};

	{
		auto sizeValue = json.find("size");
		if (sizeValue == json.end()) throw except::configValueMissing_ex("size");
		
		size = parseVec2(*sizeValue, "size");
	}

	{
		auto linesValue = json.find("lines");
		if (linesValue == json.end()) throw except::configValueMissing_ex("lines");
		if (!linesValue->is_array()) throw except::configValueMistyped_ex("lines", "Line[]");

		lines.clear();
		lines.reserve(linesValue->size());
		for (size_t i = 0, len = linesValue->size(); i < len; i++) {
			auto& lineValue = linesValue->at(i);
			auto& line = lines.emplace_back();
			if (!lineValue.is_object()) throw except::configValueMistyped_ex("lines[" + std::to_string(i) + "]", "Line");
			{
				auto point = lineValue.find("a");
				if (point == lineValue.end()) throw except::configValueMissing_ex("lines[" + std::to_string(i) + "].a");
				
				line.a = parseVec2(*point, "lines[" + std::to_string(i) + "].a");
			}
			{
				auto point = lineValue.find("b");
				if (point == lineValue.end()) throw except::configValueMissing_ex("lines[" + std::to_string(i) + "].b");

				line.b = parseVec2(*point, "lines[" + std::to_string(i) + "].b");
			}
		}
	}
}
