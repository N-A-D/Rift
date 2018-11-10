#pragma once

namespace rift {
	namespace agent {
		namespace details {

			// 2D space vector for positions & velocities
			template <class T>
			struct Vec2 {
				
				Vec2() : x(0), y(0) {}
				Vec2(T x, T y) : x(x), y(y) {}
				Vec2(std::pair<T, T> point) : x(point.first), y(point.second) {}

				Vec2<T>& operator+=(const Vec2<T>& u) noexcept {
					x += u.x;
					y += u.y;
					return *this;
				}

				Vec2<T>& operator-=(const Vec2<T>& u) noexcept {
					x -= u.x;
					y -= u.y;
					return *this;
				}

				Vec2<T>& operator*=(T scale) noexcept {
					x *= scale;
					y *= scale;
					return *this;
				}

				T x, y;
			};

			template <class T> Vec2<T> operator+(const Vec2<T>& u, const Vec2<T>& v) noexcept { return Vec2<T>(u) += v; }
			template <class T> Vec2<T> operator-(const Vec2<T>& u, const Vec2<T>& v) noexcept { return Vec2<T>(u) -= v; }
			template <class T> Vec2<T> operator*(const Vec2<T>& u, T scale) noexcept { return Vec2<T>(u) *= scale; }
			template <class T> Vec2<T> operator*(T scale, const Vec2<T>& u) noexcept { return u * scale; }
		}
	}
}