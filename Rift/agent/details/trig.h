#pragma once

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif // !_USE_MATH_DEFINES

#include <cmath>
#include <type_traits>
#include "vec2.h"

#ifndef M_PI 
#define M_PI 3.14159265358979323846
#endif // !M_PI 

namespace rift {
	namespace details {

		// Converts radians to degrees
		template <class T>
		typename std::enable_if<std::is_floating_point<T>::value, T>::type
			degrees(T radians) noexcept {
			return radians * static_cast<T>(180.0) / static_cast<T>(M_PI);
		}

		// Converts degrees to radians
		template <class T>
		typename std::enable_if<std::is_floating_point<T>::value, T>::type
			radians(T degrees) noexcept {
			return degrees * static_cast<T>(M_PI) / static_cast<T>(180.0);
		}

		// Return u rotated angle degrees
		template <class T>
		Vec2<T> rotate(const Vec2<T>& u, T angle) noexcept {
			auto rad = radians(angle);
			auto x = u.x * cos(rad) - u.y * sin(rad);
			auto y = u.x * sin(rad) + u.y * cos(rad);
			return Vec2<T>(x, y);
		}

		// Returns the angle between two vectors u and v
		template <class T>
		T angle_btwn(const Vec2<T>& u, const Vec2<T>& v) {
			return degrees(acos(dot(norm(u), norm(v))));
		}

	}
}