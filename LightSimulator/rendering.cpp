#include "pch.h"
#include "rendering.h"

color_t photon_t::calculateColor() {
	if (wavelength == 0) return color_t(1, 1, 1);
	extent_t gamma = 0.80;

	extent_t factor;
	extent_t red, green, blue;

	if ((wavelength >= 380) && (wavelength < 440)) {
		red = -(wavelength - 440) / (440.0 - 380.0); // The decimal zero tells the compiler to use doubles instead of integers
		green = 0.0;
		blue = 1.0;
	} else if ((wavelength >= 440) && (wavelength < 490)) {
		red = 0.0;
		green = (wavelength - 440) / (490.0 - 440.0);
		blue = 1.0;
	} else if ((wavelength >= 490) && (wavelength < 510)) {
		red = 0.0;
		green = 1.0;
		blue = -(wavelength - 510) / (510.0 - 490.0);
	} else if ((wavelength >= 510) && (wavelength < 580)) {
		red = (wavelength - 510) / (580.0 - 510);
		green = 1.0;
		blue = 0.0;
	} else if ((wavelength >= 580) && (wavelength < 645)) {
		red = 1.0;
		green = -(wavelength - 645) / (645.0 - 580.0);
		blue = 0.0;
	} else if ((wavelength >= 645) && (wavelength < 781)) {
		red = 1.0;
		green = 0.0;
		blue = 0.0;
	} else {
		red = 0.0;
		green = 0.0;
		blue = 0.0;
	};

	// Let the intensity fall off near the vision limits

	if ((wavelength >= 380) && (wavelength < 420)) {
		factor = 0.3 + 0.7 * (wavelength - 380) / (420.0 - 380);
	} else if ((wavelength >= 420) && (wavelength < 701)) {
		factor = 1.0;
	} else if ((wavelength >= 701) && (wavelength < 781)) {
		factor = 0.3 + 0.7 * (780 - wavelength) / (780.0 - 700);
	} else {
		factor = 0.0;
	};

	return color_t(
		std::pow(red * factor, gamma),
		std::pow(green * factor, gamma),
		std::pow(blue * factor, gamma)
	);
}


void renderWorker_t::executeStep() {
	auto dist = std::uniform_int_distribution<size_t>(0, pixels.size() - 1);
	// Delete photons
	for (size_t i = photons.size() - 1; i >= 0 && i != -1; i--) {
		auto& photon = photons[i];
		if (
			// Check if the photon is outside the area
			photon.position.x < 0 ||
			photon.position.y < 0 ||
			photon.position.x >= space.size.x ||
			photon.position.y >= space.size.y ||
			photon.intensity < 0.001
			)
			photons.erase(photons.begin() + i);
	}

	for (size_t i = 0, len = photons.size(); i < len; i++) {
		auto& photon = photons[i];

		// Move photons

		auto oldPos = photon.position;

		auto dist = space.getGlobalMinDist(photon.position);
		if (dist < 0.1) photon.intensity = 0;
		if (dist > 1) dist = 1;

		photon.position = photon.position + (photon.direction * dist);

		/// Draw photons
		// Old x pos
		int ox = (int)(oldPos.x / (extent_t)space.size.x * (extent_t)width);
		// Old y pos
		int oy = (int)(oldPos.y / (extent_t)space.size.y * (extent_t)height);
		// New x pos
		int sx = (int)(photon.position.x / (extent_t)space.size.x * (extent_t)width);
		// New y pos
		int sy = (int)(photon.position.y / (extent_t)space.size.y * (extent_t)height);

		int x, y, dx, dy, dx1, dy1, px, py, xe, ye;
		dx = sx - ox;
		dy = sy - oy;
		dx1 = std::abs(dx);
		dy1 = std::abs(dy);
		px = 2 * dy1 - dx1;
		py = 2 * dx1 - dy1;
		if (dy1 <= dx1) {
			if (dx >= 0) {
				x = ox;
				y = oy;
				xe = sx;
			} else {
				x = sx;
				y = sy;
				xe = ox;
			}
			for (int i = 0; x < xe; i++) {
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
				if (x >= (int)width || y >= (int)height || x < 0 || y < 0) break;
				pixels[x + y * width] = pixels[x + y * width] + (photon.calculateColor() * photon.intensity);
			}
		} else {
			if (dy >= 0) {
				x = ox;
				y = oy;
				ye = sy;
			} else {
				x = sx;
				y = sy;
				ye = oy;
			}
			for (int i = 0; y < ye; i++) {
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
				if (x >= (int)width || y >= (int)height || x < 0 || y < 0) break;
				pixels[x + y * width] = pixels[x + y * width] + (photon.calculateColor() * photon.intensity);
			}
		}
	}

	photonsRemaining.store(photons.size());
}

static vec2_t getRandomDir(std::mt19937& randomSource) {
	vec2_t ret;
	auto dist = std::uniform_real_distribution<extent_t>(-1, 1);
	do {
		ret = vec2_t(dist(randomSource), dist(randomSource));
	} while (ret.length() > 1);

	return ret.normalize();
}

void renderWorker_t::execute() {
	// Initialize photons
	photons.resize(photonNum);
	size_t size = photons.size();
	photonsRemaining.store(size);
	// Distribute photons
	/* The index of the last distributed photon */
	size_t last = 0;
	for (auto& spawner : space.spawners) {
		auto amount = (size_t)(spawner.ratio * photonNum);
		if (spawner.type == spawner_t::type_e::square) {
			auto xDist = std::uniform_real_distribution(spawner.pos.x - spawner.size.x / 2, spawner.pos.x + spawner.size.x / 2);
			auto yDist = std::uniform_real_distribution(spawner.pos.y - spawner.size.y / 2, spawner.pos.y + spawner.size.y / 2);
			auto wavelenghtDist = std::uniform_real_distribution(spawner.wavelenghtMin, spawner.wavelenghtMax);

			for (size_t i = last; i < amount; i++) {
				photons[i].position = vec2_t(xDist(randomSource), yDist(randomSource));
				photons[i].wavelength = wavelenghtDist(randomSource);
			}
		}
		last += amount;
	}


	for (size_t i = 0; i < size; i++) {
		photons[i].direction = getRandomDir(randomSource);
	}

	// Initialize the pixels
	pixels.resize(width * height);
	std::fill(pixels.begin(), pixels.end(), color_t());
	// Start render loop
	while (!photons.empty()) {
		executeStep();
	}
}

void renderWorker_t::startThread() {
	thread = std::thread([this]() {
		execute();
	});
}

renderWorker_t::renderWorker_t(const space_t& space, size_t photonNum, size_t width, size_t height) : photonNum(photonNum), width(width), space(space), height(height) {
	// All alocations will be done on a separate thread in execute()
};

static std::random_device randomDevice;

void batchController_t::startRendering(const space_t& space, size_t photonNum, size_t threadCount) {
	/* The amount of photons for one worker */
	auto countForOne = photonNum / threadCount;

	workers.resize(threadCount);
	for (auto& worker : workers) {
		worker = std::make_unique<renderWorker_t>(space, countForOne, width, height);
		worker->sourceRandom(randomDevice);
	}

	for (auto& worker : workers) {
		worker->startThread();
	}

	initialWorkerNum = workers.size();
}

bool batchController_t::isDone() {
	return workers.empty();
}

double batchController_t::update() {
	double remaining = 0;
	// Calculating the percentage of photons remaining
	for (auto& worker : workers) {
		if (!worker) continue;
		remaining += (double)worker->getPhotonsRemaining() / (double)worker->getPhotonNum();
	}

	remaining /= initialWorkerNum;
	/// Removing the finished workers
	auto iter = std::remove_if(workers.begin(), workers.end(), [this](std::unique_ptr<renderWorker_t>& worker) {
		if (!worker) return true;
		if (worker->isDone()) {
			worker->join();
			auto myPixels = (extent_t*)pixels.data();
			auto theirPixels = (extent_t*)worker->pixels.data();
			for (size_t i = 0, len = pixels.size() * 3; i < len; i++) {
				myPixels[i] += theirPixels[i];
			}
			pixelsDirty = true;
			return true;
		} else return false;
	});

	if (iter != workers.end()) workers.erase(iter);
	// Invert the remaining because it contains the percentage of photons remaining and we want the percetage of photons done
	return 1 - remaining;
}

void batchController_t::resize(size_t width, size_t height) {
	if (width == this->width && height == this->height)
		return;
	this->width = width;
	this->height = height;

	pixels.resize(width * height);
	clear();
}

void batchController_t::drawPreview(SDL_Surface* surface, const SDL_Rect& rect, double zoom) {
	if (!pixels.empty()) {
		if (pixelsDirty || !cacheSurface || cacheSurface->w != rect.w || cacheSurface->h != rect.h) {
			cacheSurface = draw(rect.w, rect.h, 1);
		}

		auto copy = rect;
		sdlhelp::handleSDLError(SDL_BlitSurface(cacheSurface.get(), nullptr, surface, &copy));
	}
	pixelsDirty = false;
}

sdlhelp::unique_surface_ptr batchController_t::draw(int w, int h, extent_t exp) {
	if (w <= 0) w = (int)width;
	if (h <= 0) h = (int)height;
	sdlhelp::unique_surface_ptr surface;
	surface.reset(sdlhelp::handleSDLError(SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0)));
	auto surfacePtr = surface.get();
	auto myPixels = pixels.data();
	double xZoom = (double)w / (double)width;
	double yZoom = (double)h / (double)height;
	for (int y = 0; y < w; y++)
		for (int x = 0; x < h; x++) {
			auto pX = (size_t)((double)x / xZoom);
			auto pY = (size_t)((double)y / yZoom);
			auto index = pX + pY * width;
			auto r = myPixels[index].r * 255;
			auto g = myPixels[index].g * 255;
			auto b = myPixels[index].b * 255;
			if (r >= 256) r = 255;
			if (g >= 256) g = 255;
			if (b >= 256) b = 255;
			SDL_Rect target = { x, y, 1, 1 };
			SDL_FillRect(surfacePtr, &target, SDL_MapRGB(surface->format, (Uint8)r, (Uint8)g, (Uint8)b));
		}
	return surface;
}

void batchController_t::clear() {
	std::fill(pixels.begin(), pixels.end(), color_t());
	pixelsDirty = true;
}
