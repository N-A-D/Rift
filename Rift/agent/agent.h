#pragma once
#ifndef _RIFT_AUTONOMOUS_AGENT_SYSTEM_
#define _RIFT_AUTONOMOUS_AGENT_SYSTEM_

#define _USE_MATH_DEFINES
#include <cmath>
#include "../rift.h"
#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <numeric>
#include <list>
#include <array>

namespace rift {

	// namespace is not to be used
	namespace impl {
		
		template <class T>
		struct Vec2 {
			Vec2() : x(0), y(0) {}
			Vec2(T x, T y)
				: x(x), y(y) {}

			Vec2<T>& operator+=(const Vec2<T>& v) noexcept {
				x += v.x;
				y += v.y;
				return *this;
			}

			Vec2<T>& operator-=(const Vec2<T>& v) noexcept {
				x -= v.x;
				y -= v.y;
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

		using Vec2f = Vec2<float>;
		using Vec2d = Vec2<double>;

		// Converts radians to degrees
		template <class T>
		T degrees(T radians) noexcept;

		// Converts degrees to radians 
		template <class T>
		T radians(T degrees) noexcept;

		// Returns the length of u
		template <class T>
		T length(const Vec2<T>& u) noexcept;

		// Returns the length squared of u
		template <class T>
		T length_sq(const Vec2<T>& u) noexcept;

		// Returns the distance between u and v
		template <class T>
		T dist_btwn(const Vec2<T>& u, const Vec2<T>& v) noexcept;

		// Returns the distance squared between u and v
		template <class T>
		T dist_btwn_sq(const Vec2<T>& u, const Vec2<T>& v) noexcept;

		// Returns the angle between u and v
		template <class T>
		T angle_btwn(const Vec2<T>& u, const Vec2<T>& v) noexcept;

		// Returns the dot product of u and v
		template <class T>
		T dot(const Vec2<T>& u, const Vec2<T>& v) noexcept;

		// Returns a normalized u
		template <class T>
		Vec2<T> norm(const Vec2<T>& u) noexcept;

		// Returns a truncated to 'length' u
		template <class T>
		Vec2<T> trunc(const Vec2<T>& u, T length) noexcept;

		// Returns a rotated u by 'angle' degrees
		template <class T>
		Vec2<T> rotate(const Vec2<T>& u, T angle) noexcept;

		// Returns a vector orthogonal to u
		// Note:
		// - The vector is +90 degrees from u
		template <class T>
		Vec2<T> ortho(const Vec2<T>& u) noexcept;

		template<class T>
		T degrees(T radians) noexcept
		{
			return radians * static_cast<T>(180.0) / static_cast<float>(M_PI);
		}

		template<class T>
		T radians(T degrees) noexcept
		{
			return degrees * static_cast<float>(M_PI) / static_cast<T>(180.0);
		}

		template<class T>
		T length(const Vec2<T>& u) noexcept
		{
			return sqrt(pow(u.x, 2) + pow(u.y, 2));
		}

		template<class T>
		T length_sq(const Vec2<T>& u) noexcept
		{
			return pow(u.x, 2) + pow(u.y, 2);
		}

		template<class T>
		T dist_btwn(const Vec2<T>& u, const Vec2<T>& v) noexcept
		{
			return sqrt(pow(v.x - u.x, 2) + pow(v.y - u.y, 2));
		}

		template<class T>
		T dist_btwn_sq(const Vec2<T>& u, const Vec2<T>& v) noexcept
		{
			return pow(v.x - u.x, 2) + pow(v.y - u.y, 2);
		}

		template<class T>
		T angle_btwn(const Vec2<T>& u, const Vec2<T>& v) noexcept
		{
			return degrees(acos(dot(norm(u), norm(v))));
		}

		template<class T>
		T dot(const Vec2<T>& u, const Vec2<T>& v) noexcept
		{
			return u.x * v.x + u.y * v.y;
		}

		template<class T>
		Vec2<T> norm(const Vec2<T>& u) noexcept
		{
			auto mag = length(u);
			if (mag == 0) throw std::runtime_error("Cannot normalize the zero vector");
			return Vec2<T>(u.x / mag, u.y / mag);
		}

		template<class T>
		Vec2<T> trunc(const Vec2<T>& u, T length) noexcept
		{
			return norm(u) * length;
		}

		template<class T>
		Vec2<T> rotate(const Vec2<T>& u, T angle) noexcept
		{
			auto rads = radians(angle);
			auto x = u.x * cos(rads) - u.y * sin(rads);
			auto y = u.x * sin(rads) + u.y * cos(rads);
			return Vec2<T>(x, y);
		}

		template<class T>
		Vec2<T> ortho(const Vec2<T>& u) noexcept
		{
			return Vec2<T>(-u.y, u.x);
		}


		// Matrix class for view transformations
		template <class T>
		class Mat2 {
		public:
			Mat2() : x11(0), x12(0), x21(0), x22(0) {}
			Mat2(T x11, T x12, T x21, T x22)
				: x11(x11), x12(x12), x21(x21), x22(x22) {}

			static Mat2<T> identity() noexcept {
				return Mat2<T>(1, 0, 0, 1);
			}

			static Mat2<T> rotation(const Vec2<T>& u, const Vec2<T>& v) noexcept {
				return Mat2<T>(u.x, u.y, v.x, v.y);
			}

			Vec2<T> operator*=(const Vec2<T>& p) noexcept {
				auto x = p.x * x11 + p.y * x12;
				auto y = p.x * x21 + p.y * x22;
				return Vec2<T>(x, y);
			}

		private:
			T x11, x12;
			T x21, x22;
		};

		template <class T>
		Vec2<T> operator*(const Mat2<T>& m, const Vec2<T>& p) noexcept { return Mat2<T>(m) *= p; }

		// Converts points in world space to entity local space
		template <class T>
		Vec2<T> convert_to_local_space(const Vec2<T>& origin, const Vec2<T>& u, const Vec2<T>& v, const Vec2<T>& p) noexcept {
			Vec2<T> translated_point = p - origin;
			Mat2<T> rotation(Mat2<T>::rotation(u, v));
			return rotation * translated_point;
		}

		// Converts points in entity local space to world space
		template <class T>
		Vec2<T> convert_to_world_space(const Vec2<T>& origin, const Vec2<T>& u, const Vec2<T>& v, const Vec2<T>& p) noexcept {
			Mat2<T> rotation(Mat2<T>::rotation(Vec2<T>(u.x, v.x), Vec2<T>(u.y, v.y)));
			Vec2<T> rotated_point = rotation * p;
			return rotated_point + origin;
		}

		// Spatial partitioner
		// TODO: add event management
		class CellSpace {
		public:

			using value_type = rift::Entity;
			using size_type = std::vector<rift::Entity>::size_type;

			// Boundary of a cell
			struct Boundary {
				Boundary() : x(0), y(0), w(0), h(0) {}
				Boundary(size_type x, size_type y, size_type w, size_type h) 
					: x(x), y(y), w(w), h(h) {}

				// Checks for boundary intersection
				bool intersects_with(const Boundary& other) const noexcept {
					if (other.x >= x + w)
						return false;
					if (other.x + other.w <= x)
						return false;
					if (other.y >= y + h)
						return false;
					if (other.y + other.h <= y)
						return false;
					return true;
				}

				// Boundary components
				size_type x, y, w, h;
			};

			// Contains entities in a boundary
			struct Cell {
				Cell(size_type x, size_type y, size_type w, size_type h)
					: boundary(x, y, w, h) {}

				// The boundaries of the cell
				Boundary boundary;

				// The entities in the cell's boundaries
				std::list<rift::Entity> members;
			};

			CellSpace(size_type world_width, size_type world_height, size_type num_cells_x, size_type num_cells_y)
				: world_width(world_width)
				, world_height(world_height)
				, num_cells_x(num_cells_x == 0 ? 1 : num_cells_x)
				, num_cells_y(num_cells_y == 0 ? 1 : num_cells_y)
			{
				size_type cell_width = world_width / num_cells_x;
				size_type cell_height = world_height / num_cells_y;

				for (size_type x = 0; x < num_cells_x; x++) {
					for (size_type y = 0; y < num_cells_y; y++) {
						grid.push_back(Cell(x * cell_width, y * cell_height, cell_width, cell_height));
					}
				}
			}

			// Returns the number of managed entities
			size_type size() const noexcept { std::accumulate(grid.begin(), grid.end(), size_type(0), [](size_type n, Cell cell) { return n + static_cast<size_type>(cell.members.size()); }); }

			// Checks if an entity is contained in the spatial partitioning
			bool contains(const rift::Entity& entity) const noexcept {
				return entity_cell_locations.find(entity.id().number()) != entity_cell_locations.end();
			}

			// Returns the cell index for a given position
			template <class T>
			size_type cell_for(const Vec2<T>& p) noexcept {
				// Formula -> column_length * column + row
				size_type index = num_cells_y * static_cast<size_type>(num_cells_x * p.x / world_width)
					              + static_cast<size_type>(num_cells_y * p.y / world_height);
				return (index >= grid.size()) ? grid.size() - 1 : index;
			}

			// Adds a new entity into the appropriate cell space
			// Note: 
			// - Asserts the entity is not already a part of the spatial partitioning
			template <class T>
			void add_entity(const rift::Entity& entity, const Vec2<T>& p) noexcept {
				assert(!contains(entity));
				auto index = cell_for(p);
				grid.at(index).members.push_back(entity);
				entity_cell_locations.insert(std::make_pair(entity.id().number(), index));
			}

			// Removes an entity from the its associated cell space
			// Note:
			// - Asserts the entity is a part of the spatial partitioning
			template <class T>
			void remove(const rift::Entity& entity, const Vec2<T>& p) noexcept {
				assert(contains(entity));
				auto index = cell_for(p);
				grid.at(index).members.remove(entity);
				entity_cell_locations.erase(entity.id().number());
			}

			// Update the position of an entity
			// Note:
			// - Asserts the entity is a part of the spatial partitioning
			template <class T>
			void update_entity(const Entity& entity, const Vec2<T>& op, const Vec2<T>& p) noexcept {
				assert(contains(entity));
				auto old_cell = cell_for(op);
				auto new_cell = cell_for(p);
				if (old_cell != new_cell) {
					grid.at(old_cell).members.remove(entity);
					grid.at(new_cell).members.push_back(entity);
					entity_cell_locations.at(entity.id().number()) = new_cell;
				}
			}


		private:

			// Grid of cells stored in column major form
			std::vector<Cell> grid;

			// Dimensions of the cell space
			size_type world_width, world_height;

			// Number of cells vertically and horizontally
			size_type num_cells_x, num_cells_y;

			// Address table from entities to cell locations
			std::unordered_map<std::uint64_t, size_type> entity_cell_locations;
		};
	}

	namespace agent {


		enum class Behaviour {

		};

		struct AutonomousAgent : public Component<AutonomousAgent> {

		};

		class AutonomousAgentSystem : public System <AutonomousAgentSystem> {
		public:
			AutonomousAgentSystem() = default;

		private:
			impl::CellSpace world;
		};

	}
}

#endif // !_RIFT_AUTONOMOUS_AGENT_SYSTEM_

