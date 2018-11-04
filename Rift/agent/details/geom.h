#pragma once

#include "vec2.h"
#include <stdexcept>

namespace rift {
	namespace details {

		// Returns the length of u
		template <class T>
		T length(const Vec2<T>& u) noexcept {
			return sqrt(pow(u.x, 2) + pow(u.y, 2));
		}

		// Returns the length of u squared
		template <class T>
		T length_sq(const Vec2<T>& u) noexcept {
			return pow(u.x, 2) + pow(u.y, 2);
		}

		// Returns the distance between vectors u and v
		template <class T>
		T dist_btwn(const Vec2<T>& u, const Vec2<T>& v) noexcept {
			return sqrt(pow(v.x - u.x, 2) + pow(v.y - u.y, 2));
		}

		// Returns the distance between vectors u and v squared
		template <class T>
		T dist_btwn_sq(const Vec2<T>& u, const Vec2<T>& v) noexcept {
			return pow(v.x - u.x, 2) + pow(v.y - u.y, 2);
		}

		// Returns the dot product of vectors u and v
		template <class T>
		T dot(const Vec2<T>& u, const Vec2<T>& v) noexcept {
			return u.x * v.x + u.y * v.y;
		}

		// Returns a normalized u
		template <class T>
		Vec2<T> norm(const Vec2<T>& u) {
			auto mag = length(u);
			if (mag == 0) throw std::runtime_error("Cannot normalize the zero vector");
			return Vec2<T>(u.x / mag, u.y / mag);
		}

		// Returns a truncated u
		template <class T>
		Vec2<T> trunc(const Vec2<T>& u, T length) {
			return norm(u) * length;
		}

		// Returns a vector orthogonal to u
		// Note:
		// - The vector will be +90 degrees from u
		template <class T>
		Vec2<T> ortho(const Vec2<T>& u) noexcept {
			return Vec2<T>(-u.y, u.x);
		}

	}
}