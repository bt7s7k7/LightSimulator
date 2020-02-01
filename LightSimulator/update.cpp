#include "pch.h"
#include "update.h"
#include "exceptions.h"
#include "rendering.h"

constexpr char WINDOW_TITLE[] = "Light Simulator ";

void update(space_t& space) {

	std::atomic<bool>* threadActive = nullptr;
	std::deque<std::string> commands;
	std::mutex commandsMutex;
	std::filesystem::path lastOpenFile;
	batchController_t controller;
	bool screenDirty = true;
	double pixelsPerUnit = 10;

	auto commandThread = std::thread([&]() {
		auto uniqueThreadActive = std::make_unique<std::atomic<bool>>(true);
		threadActive = uniqueThreadActive.get();

		while (threadActive->load()) {
			std::string command;
			std::getline(std::cin, command);
			if (!threadActive->load()) break;

			{
				std::unique_lock<std::mutex> lock(commandsMutex);
				commands.push_back(command);
			}
		}
		spdlog::info("Command thread exited");
		threadActive = nullptr;
	});

	sdlhelp::unique_window_ptr window;
	window.reset(sdlhelp::handleSDLError(SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 500, SDL_WINDOW_RESIZABLE)));

	spdlog::info("Window initialized");

	auto surface = sdlhelp::handleSDLError(SDL_GetWindowSurface(window.get()));

	spdlog::info("Event loop starting");

	while (true) {
		auto start = std::chrono::high_resolution_clock::now();

		SDL_Event event;
		// Polling events
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_WINDOWEVENT) {
				if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
					goto eventLoopExit;
				} else if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
					surface = sdlhelp::handleSDLError(SDL_GetWindowSurface(window.get()));
					screenDirty = true;
				}
			} else if (event.type == SDL_MOUSEMOTION) {
				screenDirty = true;
			}
		}
		// Updating the batch controller, if there is any
		if (!controller.isDone()) {
			// The title is set to display the percentage of photons done
			SDL_SetWindowTitle(window.get(), (WINDOW_TITLE + std::to_string(controller.update() * 100) + "%").c_str());
		} else {
			SDL_SetWindowTitle(window.get(), WINDOW_TITLE);
		}
		// Listening to commands
		{
			// The command queue is on a another thread
			std::unique_lock<std::mutex> lock(commandsMutex);

			auto loadFileOnPath = [&](const std::filesystem::path& path) {
				try {
					space.loadFromFile(path);
					spdlog::info("Loaded space from file");
					lastOpenFile = path;
				} catch (const except::config_ex & err) {
					spdlog::error(err.what());
					space.clear();
				} catch (const except::fileOpenFail_ex & err) {
					spdlog::error(err.what());
					space.clear();
				}
				screenDirty = true;
				controller.clear();
			};

			while (!commands.empty()) {
				auto command = commands.front();
				commands.pop_front();
				if (command[0] == 'o') {
					auto path = std::filesystem::path(command.substr(1));

					loadFileOnPath(path);
				} else if (command == "r") {
					if (!lastOpenFile.empty()) {
						loadFileOnPath(lastOpenFile);
					} else {
						spdlog::error("No file was opened");
					}
				} else if (command == "q") {
					goto eventLoopExit;
				} else if (command[0] == 'p') {
					size_t number = 0;
					try {
						auto out = std::stoll(command.substr(1));
						if (out < 0) number = 0;
						else number = (size_t)out;
					} catch (const std::invalid_argument&) {
						number = 0;
					}
					if (number != 0) {
						if (!controller.isDone()) {
							spdlog::error("Currently rendering");
						} else {
							if (space.size.x == 0 || space.size.y == 0) spdlog::error("Cannot render empty space");
							else {
								spdlog::info("Preparing to render {} photons", number);

								controller.resize(
									(size_t)std::ceil(space.size.x * pixelsPerUnit),
									(size_t)std::ceil(space.size.y * pixelsPerUnit)
								);

								controller.startRendering(space, number);
							}
						}
					} else spdlog::error("Invalid number");
				} else if (command[0] == '*') {
					double number = 0;
					try {
						auto out = std::stod(command.substr(1));
						if (out < 0) number = 0;
						else number = out;
					} catch (const std::invalid_argument&) {
						number = 0;
					}
					if (number != 0) {
						pixelsPerUnit = number;
					} else spdlog::error("Invalid number");
				} else if (command[0] == 'c') {
					if (command.size() != 1) {
						try {
							std::filesystem::current_path(command.substr(1));
						} catch (const std::filesystem::filesystem_error & err) {
							spdlog::error(err.what());
						}
					}
					spdlog::info("{}", std::filesystem::current_path().string());
				} else if (command[0] == 's') {
					auto path = command.substr(1);
					auto surface = controller.draw(0, 0, 1);

					try {
						IMG_SavePNG(surface.get(), path.c_str());
					} catch (const sdlhelp::SDLException& err) {
						spdlog::error("{}", err.what());
					}
				} else {
					spdlog::error("Unknown command");
				}
			}
		}

		SDL_Point mousePos;
		auto mouseState = SDL_GetMouseState(&mousePos.x, &mousePos.y);

		if (screenDirty || controller.arePixelsDirty()) {
			SDL_FillRect(surface, nullptr, 0);
			space.drawDebug(surface, (mouseState & SDL_BUTTON_LMASK) > 0, mousePos, [&controller, surface, pixelsPerUnit](const SDL_Rect& rect, double zoom) {
				controller.drawPreview(surface, rect, zoom / (double)pixelsPerUnit);
			});
			SDL_UpdateWindowSurface(window.get());

			screenDirty = false;
		}

		auto end = std::chrono::high_resolution_clock::now();

		auto duration = (int)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		auto delay = 5 - duration;
		if (delay > 0) {
			SDL_Delay(delay);
		}
	}
eventLoopExit:
	spdlog::info("Event loop ended");

	*threadActive = false;

	commandThread.detach();
}
