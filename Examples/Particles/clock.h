#pragma once

#include <SDL.h>
#include <cstdint>

namespace pyro {
	class Clock final {
	public:
		Clock() : current(SDL_GetTicks()) {}
		void start() { current = SDL_GetTicks(); }
		std::uint32_t get_ticks() const { return SDL_GetTicks() - current; }
	private:
		std::uint32_t current;
	};
}
