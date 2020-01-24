#pragma once
#include "pch.h"
#include "vectors.h"
#include "space.h"

struct photon_t {
	vec2_t position;
	extent_t intensity;
	vec2_t direction;
	extent_t wavelength;

	color_t calculateColor();
};

class renderWorker_t {
protected:
	std::vector<color_t> pixels;
	size_t width;
	std::vector<photon_t> photons;
	space_t* space;
	std::thread thread;

	std::atomic<size_t> photonsRemaining;
	size_t photonNum;
public:
	void executeStep();
	void execute();
	void startThread();

	inline void join() {
		thread.join();
	}

	inline size_t getPhotonsRemaining() {
		return photonsRemaining.load();
	}

	inline size_t getPhotonNum() {
		return photonNum;
	}

	renderWorker_t(space_t* space, size_t photonNum, size_t width, size_t height);
};

class batchController_t {
protected:
	std::vector<renderWorker_t> workers;
	size_t width; 
	size_t height;
	std::vector<color_t> pixels;
	bool pixelsDirty = false;

public:
	void startRendering(size_t photonNum, size_t threadCount = 4);
	bool isDone();
	/* Returns the percentage of photons simulated */
	double update();

	batchController_t(size_t width, size_t height);
};