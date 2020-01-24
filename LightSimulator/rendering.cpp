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
    
}

void renderWorker_t::execute() {
    
}

void renderWorker_t::startThread() {
    thread = std::thread([this]() {
        execute();
    });
}

renderWorker_t::renderWorker_t(space_t* space, size_t photonNum, size_t width, size_t height) : photonNum(photonNum), width(width), space(space) {
    pixels.resize(width * height);
    std::fill(pixels.begin(), pixels.end(), color_t());

};

void batchController_t::startRendering(size_t photonNum, size_t threadCount) {}

bool batchController_t::isDone() {
    return false;
}

double batchController_t::update() {
    return 0.0;
}

batchController_t::batchController_t(size_t width, size_t height) : width(width), height(height) {
    pixels.resize(width * height);
    std::fill(pixels.begin(), pixels.end(), color_t());
}
