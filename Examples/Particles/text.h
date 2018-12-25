#pragma once

#include <SDL.h>
#include <string>
#include <cassert>
#include "font.h"
#include "window.h"
#include "renderable.h"

namespace pyro {
	class Text : public Renderable {
	public:

		Text(Window::Renderer renderer, Font font, const std::string& text, SDL_Color text_color)
			: text_string(text), text_color(text_color), text_font(font), Renderable(load_text(renderer)) 
		{}

		Font font() const { return text_font; }
		std::string text() const { return text_string; }
		SDL_Color color() const { return text_color; }

	private:

		SDL_Texture* load_text(Window::Renderer renderer) {
			assert(text_font);
			auto surf = TTF_RenderText_Solid(text_font, text_string.c_str(), text_color);
			assert(surf);
			auto texture = SDL_CreateTextureFromSurface(renderer, surf);
			SDL_FreeSurface(surf);
			return texture;
		}

		Font text_font;          // The font used to render the text
		std::string text_string; // The actual text
		SDL_Color text_color;    // The color used to render the text
	};
}