#pragma once
#include "pch.h"

namespace sdlhelp {

	/*
		This type executes the function D
		on operator().
	*/
	template <typename T, void (*D)(T*)>
	struct DestroyerType {
		void operator()(T* ptr) {
			D(ptr);
		}
	};

	using unique_window_ptr = std::unique_ptr<SDL_Window, DestroyerType<SDL_Window, SDL_DestroyWindow>>;
	using unique_renderer_ptr = std::unique_ptr<SDL_Renderer, DestroyerType<SDL_Renderer, SDL_DestroyRenderer>>;
	using unique_surface_ptr = std::unique_ptr<SDL_Surface, DestroyerType<SDL_Surface, SDL_FreeSurface>>;
	using unique_texture_ptr = std::unique_ptr<SDL_Texture, DestroyerType<SDL_Texture, SDL_DestroyTexture>>;

#ifdef SDL_TTF_H_
	using unique_font_ptr = std::unique_ptr<TTF_Font, DestroyerType<TTF_Font, TTF_CloseFont>>;
#endif // SDL_TTF_H_

	template <typename T, void(*D)(T*)>
	struct shared_handle_t : std::shared_ptr<T> {
		using std::shared_ptr<T>::shared_ptr;
		shared_handle_t(T* ptr) : std::shared_ptr<T>::shared_ptr(ptr, D) {}

		void reset(T* ptr) {
			std::shared_ptr<T>::reset(ptr, D);
		}
	};

	using shared_window_ptr = shared_handle_t<SDL_Window, SDL_DestroyWindow>;
	using shared_renderer_ptr = shared_handle_t<SDL_Renderer, SDL_DestroyRenderer>;
	using shared_surface_ptr = shared_handle_t<SDL_Surface, SDL_FreeSurface>;
	using shared_texture_ptr = shared_handle_t<SDL_Texture, SDL_DestroyTexture>;

#ifdef SDL_TTF_H_
	using shared_font_ptr = shared_handle_t<TTF_Font, TTF_CloseFont>;
#endif // SDL_TTF_H_

	class SDLException : public std::runtime_error {
	public:
		using std::runtime_error::runtime_error;
	};

	inline int handleSDLError(int errorCode) {
		if (errorCode == -1) {
			throw SDLException("[SDL] " + std::string(SDL_GetError()));
		}

		return errorCode;
	}

	inline Sint64 handleSDLError(Sint64 errorCode) {
		if (errorCode == -1) {
			handleSDLError(-1);
		}

		return errorCode;
	}

	template <typename T>
	T* handleSDLError(T* pointer) {
		if (pointer != nullptr) {
			return pointer;
		} else {
			throw SDLException("[SDL] " + std::string(SDL_GetError()));
		}
	}

	struct ConvertedTextureData {
		unique_texture_ptr texture;
		SDL_Rect rect;

		ConvertedTextureData() : texture(nullptr), rect{ 0,0,0,0 } {};
	};

	/*
		Creates a texture from a surface, returned struct contains a rect with the texture
		dimensions, offset by [offX, offY], for easy copying
	*/
	inline ConvertedTextureData surfaceToTexture(SDL_Renderer* renderer, SDL_Surface* surface, int offX = 0, int offY = 0) {
		ConvertedTextureData ret;
		ret.texture.reset(handleSDLError(SDL_CreateTextureFromSurface(renderer, surface)));
		ret.rect = { offX, offY, surface->w, surface->h };
		return ret;
	}

	inline std::unordered_map<SDL_Surface*, ConvertedTextureData> _renderCopySurfaceCachedTextures;

	inline void renderCopySurface(SDL_Renderer* renderer, SDL_Surface* surface, int offX = 0, int offY = 0, bool dontCache = false) {
		if (dontCache) {
			auto textureData = surfaceToTexture(renderer, surface, offX, offY);
			handleSDLError(SDL_RenderCopy(renderer, textureData.texture.get(), nullptr, &textureData.rect));
		} else {
			auto iter = _renderCopySurfaceCachedTextures.find(surface);
			if (iter == _renderCopySurfaceCachedTextures.end()) {
				auto ret = _renderCopySurfaceCachedTextures.insert_or_assign(surface, surfaceToTexture(renderer, surface, offX, offY));
				iter = ret.first;
			}
			handleSDLError(SDL_RenderCopy(renderer, iter->second.texture.get(), nullptr, &iter->second.rect));
		}
	}

	inline void renderCopySurfaceAndFree(SDL_Renderer* renderer, SDL_Surface* surface, int offX = 0, int offY = 0) {
		renderCopySurface(renderer, surface, offX, offY, true);
		SDL_FreeSurface(surface);
	}

	struct textureData_t {
		/* SDL_PIXELFORMAT_* */
		Uint32 format;
		/* Actual access to the texture (one of the SDL_TextureAccess values) */
		int access;
		int w;
		int h;
	};

	inline textureData_t queryTexture(SDL_Texture* texture) {
		textureData_t ret;
		handleSDLError(SDL_QueryTexture(texture, &ret.format, &ret.access, &ret.w, &ret.h));
		return ret;
	}

#ifdef SDL_TTF_H_

	struct font_t {
		std::vector<Uint8> data;
		std::unordered_map<int, unique_font_ptr> loadedSizes;

		TTF_Font* operator[](int size) {
			if (data.size() == 0) {
				throw std::runtime_error("Font data not initialized. // Forgot to call Init()?");
			}

			auto iter = loadedSizes.find(size);
			if (iter == loadedSizes.end()) {
				auto rwops = handleSDLError(SDL_RWFromMem(data.data(), data.size()));
				auto fontPtr = handleSDLError(TTF_OpenFontRW(rwops, 1, size));
				auto [niter, success] = loadedSizes.try_emplace(size, fontPtr);
				iter = niter;
			}
			return iter->second.get();
		}

		font_t& Init(std::string name, const std::filesystem::path& exePath) {
			if (data.size() > 0) return *this;
			auto rwops = handleSDLError(SDL_RWFromFile((exePath.parent_path() / (name + ".ttf")).string().data(), "rb"));

			data.resize(static_cast<std::size_t>(handleSDLError(SDL_RWsize(rwops))));

			if (SDL_RWread(rwops, data.data(), data.size(), 1) <= 0) {
				handleSDLError(-1);
			}

			return *this;
		}
	};
	/*
		This class is used for font loading, it loads caches and creates TTF_Font-s automaticaly.
	*/
	class fontloader {
		std::unordered_map<std::string, font_t> fonts = {};
		std::filesystem::path fontFolder;
	public:

		inline TTF_Font* getOrLoadFont(int size, std::string name) {
			return fonts[name].Init(name, fontFolder)[size];
		}

		inline fontloader(const std::filesystem::path& fontFolder_) : fontFolder(fontFolder_) {}
	};





#endif

}