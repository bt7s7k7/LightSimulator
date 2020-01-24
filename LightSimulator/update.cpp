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
	std::unique_ptr<batchController_t> controller;

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
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_WINDOWEVENT) {
				if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
					goto eventLoopExit;
				} else if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
					surface = sdlhelp::handleSDLError(SDL_GetWindowSurface(window.get()));
				}
			}
		}

		if (controller != nullptr) {
			SDL_SetWindowTitle(window.get(), (WINDOW_TITLE + std::to_string(controller->update() * 100) + "%").c_str());
			if (controller->isDone()) {
				SDL_SetWindowTitle(window.get(), WINDOW_TITLE);
				controller.reset();
			}
		}

		{
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
					} catch (const std::invalid_argument &) {
						number = 0;
					}
					if (number != 0) {
						if (controller != nullptr) {
							spdlog::error("Currently rendering");
						} else {
							spdlog::info("Preparing to render {} photons", number);

							controller.reset(new batchController_t(
								(size_t)std::floor(space.size.x * 10),
								(size_t)std::floor(space.size.y * 10)
							));

							controller->startRendering(number);
						}
					} else spdlog::error("Invalid number");
				} else {
					spdlog::error("Unknown command");
				}
			}
		}

		SDL_Point mousePos;
		auto mouseState = SDL_GetMouseState(&mousePos.x, &mousePos.y);

		SDL_FillRect(surface, nullptr, 0);

		space.drawDebug(surface, (mouseState & SDL_BUTTON_LMASK) > 0, mousePos);

		SDL_UpdateWindowSurface(window.get());
	}
eventLoopExit:
	spdlog::info("Event loop ended");

	*threadActive = false;

	commandThread.detach();
}
