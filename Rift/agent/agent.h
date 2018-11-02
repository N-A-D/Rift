#pragma once
#ifndef _RIFT_AUTONOMOUS_AGENT_SYSTEM_
#define _RIFT_AUTONOMOUS_AGENT_SYSTEM_

#define _USE_MATH_DEFINES
#include <cmath>
#include "../rift.h"
#include <stdexcept>
#include <array>

namespace rift {

	// Implementation namespace - not to be used by clients
	namespace math {

		// 2D vector for positions and directions
		template <class T>
		struct Vec2 {
			Vec2()
				: x(0), y(0) {}
			Vec2(T x, y)
				: x(x), y(y) {}

			Vec2<T>& operator+=(const Vec2<T>& v) noexcept {
				x += v.x;
				y += y.x;
				return *this;
			}

			Vec2<T>& operator-=(const Vec2<T>& v) noexcept {
				x -= v.x;
				y -= y.x;
				return *this;
			}

			Vec2<T>& operator*=(T scale) noexcept {
				x *= scale;
				y *= scale;
				return *this;
			}

			Vec2<T>& operator/=(T scale) {
				if (scale == 0.0) throw std::runtime_error("Division by zero!");
				x /= scale;
				y /= scale;
				return *this;
			}

			T x, y;
		};
		
		// Converts degrees to radians
		template <class T>
		T radians(T degrees) noexcept {
			return degrees * static_cast<T>(M_PI) / static_cast<T>(180.0);
		}

		// Converts radians to degrees
		template <class T>
		T degrees(T radians) noexcept {
			return radians * static_cast<T>(180.0) / static_cast<T>(M_PI);
		}

		// Returns the length of a vector u
		template <class T>
		T length(const Vec2<T>& u) noexcept {
			return sqrt(pow(u.x, 2) + pow(u.y, 2));
		}

		// Returns the length squared of a vector u
		template <class T>
		T length_sq(const Vec2<T>& u) noexcept {
			return pow(u.x, 2) + pow(u.y, 2);
		}

		// Returns the distance between vectors u and v
		template <class T>
		T dist_btwn(const Vec2<T>& u, const Vec2<T>& v) noexcept {
			return sqrt(pow(v.x - u.x, 2) + pow(v.y - u.y, 2));
		}

		// Returns the distance squared between vectors u and v
		template <class T>
		T dist_btwn_sq(const Vec2<T>& u, const Vec2<T>& v) noexcept {
			return pow(v.x - u.x, 2) + pow(v.y - u.y, 2);
		}

		// Returns the dot product between u and v
		template <class T>
		T dot(const Vec2<T>& u, const Vec2<T>& v) noexcept {
			return u.x * v.x + u.y * v.y;
		}

		// Returns a rotated u by theta degrees
		template <class T>
		Vec2<T> rotate(const Vec2<T>& u, T theta) noexcept {
			auto rads = radians(theta);
			auto x = u.x * cos(rads) - u.y * sin(rads);
			auto y = u.x * sin(rads) + u.y * cos(rads);
			return Vec2<T>(x, y);
		}

		// Returns a normalized vector u
		template <class T>
		Vec2<T> norm(const Vec2<T>& u) {
			auto mag = length(u);
			if (mag == 0) throw std::runtime_error("Cannot normalize the zero vector!");
			return Vec2<T>(u.x / mag, u.y / mag);
		}

		// Returns the angle in degrees between u and v
		template <class T>
		T angle_between(const Vec2<T>& u, const Vec2<T>& v) {
			return degrees(acos(dot(norm(u), norm(v)));
		}

		// Truncates the length of the vector u to length
		template <class T>
		Vec2<T> trunc(const Vec2<T>& u, T length) {
			auto v = norm(u);
			v *= size;
			return v;
		}

		// Returns a vector orthogonal to u
		// Note:
		// - The vector is +90 degrees from u
		template <class T>
		Vec2<T> ortho(const Vec2<T>& u) noexcept {
			return Vec2<T>(-u.y, u.x);
		}

		using Vec2f = Vec2<float>;
		using Vec2d = Vec2<double>;
		
		// Matrix class specifically used for view transformations
		template <class T>
		class Mat3 {
		public:

			// Returns the identity matrix
			static Mat3<T> identity() noexcept;

			// Returns a translation matrix
			static Mat3<T> translation(const Vec2<T>& p) noexcept;

			// Returns a rotation matrix
			static Mat3<T> rotation(const Vec2<T>& u, const Vec2<T>& v) noexcept;

		private:
		};

		// Transforms a point from world space to local space
		// Note:
		// - xaxis and yaxis form the basis for the local space centered at origin
		template <class T>
		Vec2<T> point_to_local_space(const Vec2<T>& origin, const Vec2<T>& xaxis, const Vec2<T>& yaxis, const Vec2<T>& point) noexcept;

		// Transforms a point from local space to world space
		// Note:
		// - xaxis and yaxis form the basis for the local space centered at origin 
		template <class T>
		Vec2<T> point_to_world_space(const Vec2<T>& origin, const Vec2<T>& xaxis, const Vec2<T>& yaxis, const Vec2<T>& point) noexcept;

	}

	enum class BehaviorTypes {

	};

	struct AutonomousAgent : Component<AutonomousAgent> {

	};

	struct AutonomousAgentSystem : public System<AutonomousAgentSystem> {

	};
}

#endif // !_RIFT_AUTONOMOUS_AGENT_SYSTEM_

