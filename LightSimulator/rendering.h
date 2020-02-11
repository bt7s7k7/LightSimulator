#pragma once
#include "pch.h"
#include "vectors.h"
#include "space.h"
#include "exceptions.h"

struct photon_t {
	vec2_t position;
	vec2_t direction;
	color_t color;
	const shape_t* lastCollision = nullptr;
};

class renderWorker_t {
protected:
	size_t width;
	size_t height;
	std::vector<photon_t> photons;
	const space_t& space;
	std::thread thread;

	std::atomic<size_t> photonsRemaining;
	size_t photonNum;
	std::mt19937 randomSource;

	/* Rendering step. Called from execute() */
	void executeStep();
public:
	std::vector<color_t> pixels;
	/* Allocates all resources and runs the render loop. The code that should run on a separate thread. */
	void execute();
	void startThread();

	inline void join() {
		if (thread.joinable()) thread.join();
	}

	inline size_t getPhotonsRemaining() const {
		return photonsRemaining.load();
	}

	inline size_t getPhotonNum() const {
		return photonNum;
	}

	inline bool isDone() const {
		return photonsRemaining == 0;
	}

	inline void sourceRandom(std::random_device& device) {
		randomSource = std::mt19937(device());
	}

	renderWorker_t(const space_t& space, size_t photonNum, size_t width, size_t height);
};

class batchController_t {
protected:
	std::vector<std::unique_ptr<renderWorker_t>> workers;
	size_t width = 0;
	size_t height = 0;
	std::vector<color_t> pixels;
	bool pixelsDirty = false;
	/* Used to calculate the percentage of photons done */
	size_t initialWorkerNum = 0;
	sdlhelp::unique_surface_ptr cacheSurface;
	extent_t multiplier = 0.01;

public:
	void startRendering(const space_t& space, size_t photonNum, size_t threadCount = 4);
	bool isDone();
	/* Returns the percentage of photons simulated */
	double update();
	inline bool arePixelsDirty() { return pixelsDirty; };
	void drawPreview(SDL_Surface* surface, const SDL_Rect& rect, double zoom);
	sdlhelp::unique_surface_ptr draw(int w, int h);
	inline void setMultiplier(extent_t value) {
		multiplier = value;
		pixelsDirty = true;
	};

	void resize(size_t width, size_t height);
	void clear();
};