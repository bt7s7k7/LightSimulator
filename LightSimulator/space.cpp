#include "pch.h"
#include "space.h"
#include "exceptions.h"

void space_t::drawDebug(SDL_Surface* surface, bool drawMouse, const SDL_Point& mousePos, std::function<void(const SDL_Rect&, double)> preDrawCallback) const {
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
	// Drawing border around draw area
	shapes::square(surface, SDL_Rect{ targetSpace.x - 1, targetSpace.y - 1, targetSpace.w + 2, targetSpace.h + 2 }, SDL_Color{ 255, 255, 255, 255 }, false);

	preDrawCallback(targetSpace, zoom);

	auto localToScreen = [&](const vec2_t& point) -> SDL_Point {
		return ((point * zoom) + vec2_t(targetSpace.x, targetSpace.y));
	};

	auto screenToLocal = [&](const SDL_Point& point) -> vec2_t {
		return (vec2_t(point) - vec2_t(targetSpace.x, targetSpace.y)) * (1 / zoom);
	};
	// Drawing lines
	for (auto& line : objectHolder_t<line_t>::items) {
		shapes::line(surface, localToScreen(line.a), localToScreen(line.b), SDL_Color{ 0,255,0,255 });
	}
	// Drawing line normals
	for (auto& line : objectHolder_t<line_t>::items) {
		auto middle = localToScreen((line.a + line.b) * 0.5);
		SDL_Point offset = line.getNormal(vec2_t());

		shapes::line(surface, SDL_Point{ middle.x + offset.x, middle.y + offset.y }, SDL_Point{ middle.x + offset.x * 5, middle.y + offset.y * 5 }, SDL_Color{ 255, 0, 0, 255 });
	}
	// Drawing min dist from mouse and normal of the closest shape
	if (drawMouse && !objectHolder_t<line_t>::items.empty()) {
		auto worldPos = screenToLocal(mousePos);
		auto closest = getClosestShape(worldPos);
		shapes::circle(surface, mousePos, (int)std::floor(closest.second * zoom), SDL_Color{ 0, 255, 255, 255 }, false);
		SDL_Point normal = closest.first->getNormal(worldPos) * 5;
		// Normal line drawing
		shapes::line(surface, mousePos, SDL_Point{ mousePos.x + normal.x, mousePos.y + normal.y }, SDL_Color{ 0, 255, 255, 255 });
	}
	// Drawing spawners
	for (auto& spawner : spawners) {
		auto pos = localToScreen(spawner.pos);
		constexpr SDL_Color SPAWNER_COLOR = { 255, 255, 0, 255 };
		if (spawner.type == spawner_t::type_e::square) {
			SDL_Point size = spawner.size * zoom;
			shapes::square(surface, SDL_Rect{ pos.x - size.x / 2, pos.y - size.y / 2, size.x, size.y }, SPAWNER_COLOR, false);
		} else if (spawner.type == spawner_t::type_e::circle) {
			int radius = (int)(spawner.size.x * zoom);
			shapes::circle(surface, pos, radius, SPAWNER_COLOR, false);
		}
		if (!spawner.direction.isZero()) {
			auto end = localToScreen(spawner.pos + (spawner.direction * (5 / zoom)));
			shapes::line(surface, pos, end, SPAWNER_COLOR);
		}
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

	constexpr char
		VECTOR_TYPE[] = "[x : number, y : number]",
		COLOR_TYPE[] = "[r : number, g : number, b : number]",
		SIZE[] = "size",
		COLOR[] = "color",
		LINES[] = "lines",
		LINE_TYPE[] = "Line",
		POINT_A[] = "a",
		POINT_B[] = "b",
		SPAWNERS[] = "spawners",
		SPAWNER_TYPE[] = "Spawner",
		POSITION[] = "position",
		SPREAD[] = "spread",
		DIRECTION[] = "direction",
		WAVELENGTH_MIN[] = "wavelengthMin",
		WAVELENGTH_MAX[] = "wavelengthMax",
		RATIO[] = "ratio",
		RADIUS[] = "radius",
		TYPE[] = "type",
		TYPE_ENUM[] = "'square' | 'circle'",
		NUMBER_TYPE[] = "number",
		REFLECTIVITY[] = "reflectivity",
		ROUGHNESS[] = "roughness"
		;

	std::string arrStart = "[";

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
			) throw except::configValueMistyped_ex(name, VECTOR_TYPE);

		return vec2_t(value.at(0), value.at(1));
	};

	auto parseColor = [&](const nlohmann::json::value_type& value, const std::string& name) {
		if (!value.is_array()
			|| value.size() != 3
			|| !value.at(0).is_number()
			|| !value.at(1).is_number()
			|| !value.at(2).is_number()
			) throw except::configValueMistyped_ex(name, COLOR_TYPE);

		return color_t(value.at(0), value.at(1), value.at(2));
	};

	{
		auto sizeValue = json.find("size");
		if (sizeValue == json.end()) throw except::configValueMissing_ex(SIZE);

		size = parseVec2(*sizeValue, "size");
	}

	{
		auto linesValue = json.find("lines");
		if (linesValue == json.end()) throw except::configValueMissing_ex(LINES);
		if (!linesValue->is_array()) throw except::configValueMistyped_ex(LINES, LINE_TYPE + std::string("[]"));

		objectHolder_t<line_t>::items.clear();
		objectHolder_t<line_t>::items.reserve(linesValue->size());
		for (size_t i = 0, len = linesValue->size(); i < len; i++) {
			auto& lineValue = linesValue->at(i);
			auto& line = objectHolder_t<line_t>::items.emplace_back();
			std::string linesPtr = LINES + arrStart + std::to_string(i) + "]";
			if (!lineValue.is_object()) throw except::configValueMistyped_ex(linesPtr, LINE_TYPE);
			{
				auto point = lineValue.find(POINT_A);
				std::string ptr = linesPtr + "." + POINT_A;
				if (point == lineValue.end()) throw except::configValueMissing_ex(ptr);

				line.a = parseVec2(*point, ptr);
			}
			{
				auto point = lineValue.find(POINT_B);
				std::string ptr = linesPtr + "." + POINT_B;
				if (point == lineValue.end()) throw except::configValueMissing_ex(ptr);

				line.b = parseVec2(*point, ptr);
			}
			{
				auto color = lineValue.find(REFLECTIVITY);
				std::string ptr = linesPtr + "." + REFLECTIVITY;
				if (color == lineValue.end()) throw except::configValueMissing_ex(ptr);

				line.reflectivity = parseColor(*color, ptr);
			}
			{
				auto number = lineValue.find(ROUGHNESS);
				std::string ptr = linesPtr + "." + ROUGHNESS;
				if (number == lineValue.end()) line.roughness = 0;
				else line.roughness = number->get<double>();
			}
		}
	}

	{
		auto spawnersValue = json.find(SPAWNERS);
		if (spawnersValue == json.end()) throw except::configValueMissing_ex(SPAWNERS);
		if (!spawnersValue->is_array()) throw except::configValueMistyped_ex(SPAWNERS, SPAWNER_TYPE + arrStart + "]");

		spawners.clear();
		spawners.reserve(spawnersValue->size());
		for (size_t i = 0, len = spawnersValue->size(); i < len; i++) {
			auto& spawnerValue = spawnersValue->at(i);
			auto& spawner = spawners.emplace_back();
			std::string spawnerPtr = SPAWNERS + arrStart + std::to_string(i) + "]";
			if (!spawnerValue.is_object()) throw except::configValueMistyped_ex(spawnerPtr, SPAWNER_TYPE);

			{
				auto point = spawnerValue.find(POSITION);
				std::string ptr = spawnerPtr + "." + POSITION;
				if (point == spawnerValue.end()) throw except::configValueMissing_ex(ptr);

				spawner.pos = parseVec2(*point, ptr);
			}

			{
				auto typeValue = spawnerValue.find(TYPE);
				std::string ptr = spawnerPtr + "." + TYPE;
				if (typeValue == spawnerValue.end()) throw except::configValueMissing_ex(ptr);
				if (!typeValue->is_string()) throw except::configValueMistyped_ex(ptr, TYPE_ENUM);

				auto type = typeValue->get<std::string>();
				if (type == "square") {
					spawner.type = spawner_t::type_e::square;
					{
						auto point = spawnerValue.find(SIZE);
						std::string ptr = spawnerPtr + "." + SIZE;
						if (point == spawnerValue.end()) throw except::configValueMissing_ex(ptr);

						spawner.size = parseVec2(*point, ptr);
					}
				} else if (type == "circle") {
					spawner.type = spawner_t::type_e::circle;
					{
						auto numberValue = spawnerValue.find(RADIUS);
						std::string ptr = spawnerPtr + "." + RADIUS;
						if (numberValue == spawnerValue.end()) throw except::configValueMissing_ex(ptr);
						if (!numberValue->is_number()) throw except::configValueMistyped_ex(ptr, NUMBER_TYPE);

						spawner.size.x = numberValue->get<extent_t>();
					}
				} else {
					throw except::configValueMistyped_ex(ptr, TYPE_ENUM);
				}
			}

			{
				auto point = spawnerValue.find(COLOR);
				std::string ptr = spawnerPtr + "." + COLOR;
				if (point == spawnerValue.end()) throw except::configValueMissing_ex(ptr);

				spawner.color = parseColor(*point, ptr);
			}
			{
				auto numberValue = spawnerValue.find(RATIO);
				std::string ptr = spawnerPtr + "." + RATIO;
				if (numberValue == spawnerValue.end()) throw except::configValueMissing_ex(ptr);
				if (!numberValue->is_number()) throw except::configValueMistyped_ex(ptr, NUMBER_TYPE);

				spawner.ratio = numberValue->get<extent_t>();
			}
			{
				auto numberValue = spawnerValue.find(SPREAD);
				std::string ptr = spawnerPtr + "." + SPREAD;
				if (numberValue == spawnerValue.end()) spawner.spread = 1;
				else {
					if (!numberValue->is_number()) throw except::configValueMistyped_ex(ptr, NUMBER_TYPE);
					spawner.spread = numberValue->get<extent_t>();
				}
			}
			{
				auto point = spawnerValue.find(DIRECTION);
				std::string ptr = spawnerPtr + "." + DIRECTION;
				if (point == spawnerValue.end()) spawner.direction = {};
				else spawner.direction = parseVec2(*point, ptr).normalize();
			}
		}
	}

	// Normalize the spawner ratios
	auto sum = std::accumulate(spawners.begin(), spawners.end(), 0.0, [](double value, const spawner_t& spawner) {
		return value + spawner.ratio;
	});

	std::for_each(spawners.begin(), spawners.end(), [sum](spawner_t& spawner) {
		spawner.ratio /= sum;
	});
}