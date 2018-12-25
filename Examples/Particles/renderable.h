#pragma once

#include <SDL.h>
#include <tuple>
#include <memory>

namespace pyro {
	class Renderable {
	public:

		Renderable() = default;
		Renderable(const Renderable&) = default;
		Renderable& operator=(const Renderable&) = default;

		Renderable(SDL_Texture* texture) 
			: texture(texture, [](SDL_Texture* t) { SDL_DestroyTexture(t); }) 
		{ SDL_QueryTexture(texture, &texture_format, nullptr, &w, &h); }
		
		operator SDL_Texture*() const { return texture.get(); }
		operator bool() const { return texture.get(); }

		int width() const { return w; }
		int height() const { return h; }
		std::uint32_t format() const { return texture_format; }

	private:
		int w = 0, h = 0;
		std::uint32_t texture_format = 0;
		std::shared_ptr<SDL_Texture> texture = nullptr;
	};
}