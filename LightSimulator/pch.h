// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include <iostream>
// disable all kinds of warnings, it's not my fault these libraries are bad
#pragma warning( push )
#pragma warning( disable : 26812 )
#pragma warning( disable : 6387 )
#pragma warning( disable : 26498 )
#pragma warning( disable : 26495 )
#pragma warning( disable : 4275 )
#pragma warning( disable : 26439 )
#include <SDL2/SDL.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#pragma warning( pop ) 
#include <unordered_map>
#include <stdexcept>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <atomic>
#include <mutex>
#include <thread>
#include <deque>

#include "lib/SDLHelper.h"
#include "lib/surfaceShapes/surfaceShapes.h"

#endif //PCH_H
