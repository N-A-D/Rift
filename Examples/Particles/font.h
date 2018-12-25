#pragma once

#include <string>
#include <memory>
#include <SDL_ttf.h>

namespace pyro {
	class Font final {
	public:
		Font(const std::string& path, int size)
			: font(TTF_OpenFont(path.c_str(), size), [](TTF_Font* f) { TTF_CloseFont(f); }) {}
		operator TTF_Font*() const { return font.get(); }
		operator bool() const { return font.get(); }
	private:
		std::shared_ptr<TTF_Font> font;
	};
}
