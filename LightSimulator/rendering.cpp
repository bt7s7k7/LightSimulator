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

	for (size_t i = photons.size() - 1; i >= 0 && i != -1; i--) {
		auto& curr = photons[i];
		// Temp code
		photons.erase(photons.begin() + i);
		pixels[dist(randomSource)] = { 0.1, 0.1, 0.1 };
	}


	photonsRemaining.store(photons.size());
}

void renderWorker_t::execute() {
	// Initialize photons
	photons.resize(photonNum);
	photonsRemaining.store(photons.size());
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
			cacheSurface.reset(sdlhelp::handleSDLError(SDL_CreateRGBSurfaceWithFormat(surface->flags, rect.w, rect.h, surface->format->BitsPerPixel, surface->format->format)));
			auto cacheSurfacePtr = cacheSurface.get();
			auto myPixels = pixels.data();
			for (int y = 0; y < cacheSurfacePtr->h; y++)
				for (int x = 0; x < cacheSurfacePtr->w; x++) {
					auto pX = (size_t)((double)x / zoom);
					auto pY = (size_t)((double)y / zoom);
					auto index = pX + pY * width;
					int r = int(myPixels[index].r * 255);
					int g = int(myPixels[index].g * 255);
					int b = int(myPixels[index].b * 255);
					if (r >= 256) r = 255;
					if (g >= 256) g = 255;
					if (b >= 256) b = 255;
					SDL_Rect target = { x, y, 1, 1 };
					SDL_FillRect(cacheSurfacePtr, &target, SDL_MapRGB(surface->format, r, g, b));
				}
		}

		auto copy = rect;
		sdlhelp::handleSDLError(SDL_BlitSurface(cacheSurface.get(), nullptr, surface, &copy));
	}
	pixelsDirty = false;
}

void batchController_t::clear() {
	std::fill(pixels.begin(), pixels.end(), color_t());
	pixelsDirty = true;
}
