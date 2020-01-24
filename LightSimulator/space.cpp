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

	for (auto& line : objectHolder_t<line_t>::items) {
		shapes::line(surface, localToScreen(line.a), localToScreen(line.b), SDL_Color{ 0,255,0,255 });
	}
	for (auto& line : objectHolder_t<line_t>::items) {
		auto middle = localToScreen((line.a + line.b) * 0.5);
		SDL_Point offset = line.getNormal(vec2_t());

		shapes::line(surface, SDL_Point{ middle.x + offset.x, middle.y + offset.y }, SDL_Point{ middle.x + offset.x * 5, middle.y + offset.y * 5 }, SDL_Color{ 255, 0, 0, 255 });
	}

	if (drawMouse && !objectHolder_t<line_t>::items.empty()) {
		auto worldPos = screenToLocal(mousePos);
		auto closest = getClosestShape(worldPos);
		shapes::circle(surface, mousePos, (int)std::floor(closest.second * zoom), SDL_Color{ 0, 255, 255, 255 }, false);
		SDL_Point normal = closest.first->getNormal(worldPos) * 5;
		shapes::line(surface, mousePos, SDL_Point{ mousePos.x + normal.x, mousePos.y + normal.y }, SDL_Color{ 0, 255, 255, 255 });
	}
}

extent_t space_t::getGlobalMinDist(const vec2_t& point) const {
	auto min = std::numeric_limits<extent_t>::infinity();
	{
		auto dist = objectHolder_t<line_t>::getMinDist(point);
		if (dist < min) min = dist;
	}

	return min;
}

std::pair<const shape_t*, extent_t> space_t::getClosestShape(const vec2_t& point) const {
	auto min = std::numeric_limits<extent_t>::infinity();
	const shape_t* target = nullptr;
	{
		auto dist = objectHolder_t<line_t>::getClosest(point);
		if (dist.second < min) {
			min = dist.second;
			target = dist.first;
		}
	}

	return { target, min };
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

		objectHolder_t<line_t>::items.clear();
		objectHolder_t<line_t>::items.reserve(linesValue->size());
		for (size_t i = 0, len = linesValue->size(); i < len; i++) {
			auto& lineValue = linesValue->at(i);
			auto& line = objectHolder_t<line_t>::items.emplace_back();
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
