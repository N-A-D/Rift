#pragma once

#include "vec2.h"
#include "mat2x2.h"

namespace rift {
	namespace details {

		// Converts a point defined in world space into an entity's local space
		// The local space of the entity is centered at the supplied origin with
		// coordinate vectors u and v
		template <class T>
		Vec2<T> convert_to_local_space(const Vec2<T>& origin, const Vec2<T>& u, const Vec2<T>& v, const Vec2<T>& point) noexcept
		{
			auto translated_point = point - origin;
			return Mat2<T>(u.x, u.y, v.x, v.y) * translated_point;
		}

		// Converts a point defined in an entity's local space into world space
		// The local space of the entity is centered at the supplied origin with
		// coordinate vectors u and v
		template <class T>
		Vec2<T> convert_to_world_space(const Vec2<T>& origin, const Vec2<T>& u, const Vec2<T>& v, const Vec2<T>& point) noexcept
		{
			auto rotated_point = Mat2<T>(u.x, v.x, u.y, v.y) * point;
			return origin + rotated_point;
		}

	}
}