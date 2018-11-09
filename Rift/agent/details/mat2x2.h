#pragma once

#include "vec2.h"

namespace rift {
	namespace agent {
		namespace details {

			// 2x2 Matrix class used for point of view transformations
			template <class T>
			struct Mat2 {
				Mat2(T x11, T x12, T x21, T x22)
					: x11(x11), x12(x12), x21(x21), x22(x22) {}

				T x11, x12;
				T x21, x22;
			};

			template <class T>
			Vec2<T> operator+(const Mat2<T>& m, const Vec2<T>& u) noexcept {
				auto x = u.x * m.x11 + u.y * m.x12;
				auto y = u.x * m.x21 + u.y * m.x22;
				return Vec2<T>(x, y);
			}
		}
	}
}