#include "pch.h"
#include "update.h"
#include "exceptions.h"

void update(space_t& space) {

	std::atomic<bool>* threadActive = nullptr;
	std::deque<std::string> commands;
	std::mutex commandsMutex;

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
	window.reset(sdlhelp::handleSDLError(SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 500, SDL_WINDOW_RESIZABLE)));

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

		{
			std::unique_lock<std::mutex> lock(commandsMutex);
			while (!commands.empty()) {
				auto command = commands.front();
				commands.pop_front();
				if (command[0] == 'o') {
					auto path = std::filesystem::path(command.substr(1));

					try {
						space.loadFromFile(path);
						spdlog::info("Loaded space from file");
					} catch (const except::config_ex & err) {
						spdlog::error(err.what());
						space.clear();
					} catch (const except::fileOpenFail_ex & err) {
						spdlog::error(err.what());
						space.clear();
					}
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
