#pragma once
#ifndef _RIFT_AUTONOMOUS_AGENTS_UTIL_
#define _RIFT_AUTONOMOUS_AGENTS_UTIL_

#include <list>
#include <vector>
#include <random>
#include <stdexcept>
#include <functional>
#include <type_traits>
#include <unordered_map>
#include "../core/entity.h"

namespace rift {

	/// Vec2
	namespace agent {
		namespace impl {
			
			// 2D vector class for positions & velocities
			template <class T>
			struct Vec2 {
				Vec2() : x(0), y(0) {}
				Vec2(T x, T, y) : x(x), y(y) {}

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

			// Vec2 utility functions

			template <class T>
			T length(const Vec2<T>& u) noexcept { return sqrt(pow(u.x, 2) + pow(u.y, 2)); }

			template <class T>
			T length_sq(const Vec2<T>& u) noexcept { return pow(u.x, 2) + pow(u.y, 2); }

			template <class T>
			T dist_btwn(const Vec2<T>& u, const Vec2<T>& v) noexcept { return sqrt(pow(v.x - u.x, 2) + pow(v.y - u.y, 2)); }

			template <class T>
			T dist_btwn_sq(const Vec2<T>& u, const Vec2<T>& v) noexcept { return  pow(v.x - u.x, 2) + pow(v.y - u.y, 2); }

			template <class T>
			T dot(const Vec2<T>& u, const Vec2<T>& v) noexcept { return u.x * v.x + u.y * v.y; }

			template <class T>
			Vec2<T> norm(const Vec2<T>& u) {
				auto mag = length(u);
				if (mag == 0) throw std::runtime_error("Cannot normalize the zero vector!");
				return Vec2<T>(u.x / mag, u.y / mag);
			}

			template <class T>
			Vec2<T> trunc(const Vec2<T>& u, T length) { return norm(u) * length; }

			template <class T>
			Vec2<T> ortho(const Vec2<T>& u) noexcept { return Vec2<T>(-u.y, u.x); }

			template <class T>
			T angle_btwn(const Vec2<T>& u, const Vec2<T>& v) { return acos(dot(norm(u), norm(v))); }

			template <class T>
			Vec2<T> rotate(const Vec2<T>& u, T radians) noexcept { return Vec2<T>(u.x * cos(radians) - u.y * sin(radians), u.x * sin(radians) + u.y * cos(radians)); }

			using Vec2f = Vec2<float>;
			using Vec2d = Vec2<double>;

		}
	}

	/// Mat2
	namespace agent {
		namespace impl {

			// Matrix class used for view point transformations
			template <class T>
			struct Mat2 {
				Mat2(T x11, T x12, T x21, T x22)
					: x11(x11), x12(x12), x21(x21), x22(x22) {}

				T x11, x12;
				T x21, x22;
			};

			template <class T>
			Vec2<T> operator*(const Mat2<T>& m, const Vec2<T>& p) noexcept {
				auto x = p.x * m.x11 + p.y * m.x12;
				auto y = p.x * m.x21 + p.y * m.x22;
				return Vec2<T>(x, y);
			}

		}
	}

	/// Spatial partitioning
	namespace agent {
		namespace impl {
			
			class CellSpacePartition {
			public:
				
				using size_type = std::vector<rift::Entity>::size_type;

				struct Boundary {
					Boundary() : x(0), y(0), w(0), h(0) {}
					Boundary(int x, int y, int w, int h)
						: x(x), y(y), w(w), h(h) {}
					bool intersects_with(const Boundary& other) noexcept {
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
					int x, y, w, h;
				};
			
				struct Cell {
					Cell() = default;
					Cell(int x, int y, int w, int h)
						: boundary(x, y, w, h) {}
					Boundary boundary;
					std::list<rift::Entity> members;
				};

				CellSpacePartition(size_type world_width, size_type world_height, 
								   size_type row_length, size_type column_length)
					: world_width(world_width), world_height(world_height)
					, row_length(row_length), column_length(column_length)
				{
					int cell_width = static_cast<int>(world_width / row_length);
					int cell_height = static_cast<int>(world_height / column_length);

					for (int x = 0; x < row_length; x++) {
						for (int y = 0; y < column_length; y++) {
							grid.push_back(Cell(x * cell_width, y * cell_height, cell_width, cell_height));
						}
					}
				}

				// Return the number of partitioned entities
				size_type size() const noexcept { return entity_address_table.size(); }

				// Checks if an entity is contained in the partitioning
				bool contains(const rift::Entity& entity) const noexcept { 
					return entity_address_table.find(entity.id().number()) != entity_address_table.end(); 
				}

				// Returns the cell address for a given position
				template <class T>
				size_type cell_address_for(const Vec2<T>& p) const noexcept {
					size_type cell_address = column_length * static_cast<size_type>(row_length * p.x / world_width)
														   + static_cast<size_type>(column_length * p.y / world_height);
					return (cell_address >= grid.size()) ? grid.size() - 1 : cell_address;
				}

				// Insert an entity into the spatial partitioning
				template <class T>
				void insert(const rift::Entity& entity, const Vec2<T>& p) noexcept {
					assert(!contains(entity));
					auto address = cell_address_for(p);
					grid.at(address).members.push_back(entity);
					entity_address_table.insert(std::make_pair(entity.id().number(), address));
				}

				// Remove an entity from the spatial partitioning
				template <class T>
				void remove(const rift::Entity& entity) noexcept {
					assert(contains(entity));
					auto address = entity_address_table.at(entity.id().number());
					grid.at(address).members.erase(entity);
					entity_address_table.erase(entity.id().number());
				}

				// Updates the address of the entity to its new position
				template <class T>
				void update(const rift::Entity& entity, const Vec2<T>& p) noexcept {
					assert(contains(entity));
					auto old_address = entity_address_table.at(entity.id().number());
					auto new_address = cell_address_for(p);
					if (old_address != new_address) {
						grid.at(old_address.members.remove(entity));
						grid.at(new_address.members.push_back(entity));
						entity_address_table.at(entity.id().number()) = new_address;
					}
				}

				// Applies a function onto entities who are within a given proximity
				template <class T>
				void for_each_neighbor(const Vec2<T>& p, float prox_rad, std::function<void(rift::Entity e)> f) {
					Boundary bounds(static_cast<size_type>(p.x - prox_rad / 2.0f), 
									static_cast<size_type>(p.y - prox_rad / 2.0f),
									static_cast<size_type>(prox_rad),
									static_cast<size_type>(prox_rad));

					for (auto cell : grid) {
						if (bounds.intersects_with(cell.boundary)) {
							for (auto member : cell.members) {
								f(member);
							}
						}
					}
				}

			private:
				
				// Number of rows & columns
				size_type row_length, column_length;
				
				// The dimensions of the world
				size_type world_width, world_height;

				// The grid of space cells
				std::vector<Cell> grid;

				// Entity address location
				std::unordered_map<std::uint64_t, size_type> entity_address_table;

			};

		}
	}

	/// Utility functions
	namespace agent {
		namespace impl {

			// Converts points in world space to local space
			// The vectors origin, u and v form the local coordinate system to convert to
			template <class T>
			Vec2<T> convert_to_local_space(const Vec2<T>& origin, const Vec2<T>& u, const Vec2<T>& v, const Vec2<T>& p) noexcept {
				auto translated_point = p - origin;
				return Mat2<T>(u.x, u.y, v.x, v.y) * translated_point;
			}

			// Converts points in local space to world space
			// The vectors origin, u and v form the local coordinate system to convert from
			template <class T>
			Vec2<T> convert_to_world_space(const Vec2<T>& origin, const Vec2<T>& u, const Vec2<T>& v, const Vec2<T>& p) noexcept {
				auto rotated_point = Mat2<T>(u.x, v.x, u.y, v.y) * p;
				return rotated_point + origin;
			}

			// Returns a random number between 0 and 1 inclusive
			template <class T>
			typename std::enable_if<std::is_floating_point<T>::value, T>::type
				random() noexcept {
				std::random_device rd;
				std::mt19937 gen(rd());
				std::uniform_real_distribution<T> dis(static_cast<T>(0), static_cast<T>(1));
				return dis(gen);
			}

			// Returns a number between two numbers a and b inclusive, where a < b
			template <class T>
			typename std::enable_if<std::is_floating_point<T>::value, T>::type
				random_range(T min, T max) noexcept {
				assert(min <= max);
				std::random_device rd;
				std::mt19937 gen(rd());
				std::uniform_real_distribution<T> dis(min, max);
				return dis(gen);
			}

		}
	}
}

#endif // !_RIFT_AUTONOMOUS_AGENTS_UTIL_
