#pragma once

#include "vec2.h"
#include <vector>
#include <cstddef>
#include <functional>
#include <unordered_map>
#include "../../entity.h"
#include "../../utility/cache.h"

namespace rift {
	
	namespace details {

		// The space subdivision class
		// Divides space into equally sized cells
		class CellSpacePartition {
		public:

			// Defines the boundaries of a unit of space
			struct Boundary {
				Boundary(int x, int y, int w, int h)
					: x(x), y(y), w(w), h(h) {}

				bool intersects(const Boundary& other) const noexcept
				{
					if (x >= other.x + other.w)
						return false;
					if (x + w <= other.x)
						return false;
					if (y >= other.y + other.h)
						return false;
					if (y + h <= other.y)
						return false;
					return true;
				}

				int x, y, w, h;
			};

			// A unit of space that contains entities
			struct Cell {
				Cell(int x, int y, int w, int h)
					: boundary(x, y, w, h) {}
				Boundary boundary;
				rift::Cache<rift::Entity> members;
			};

			CellSpacePartition(std::size_t world_width
				, std::size_t world_height
				, std::size_t row_length
				, std::size_t column_length)
				: world_width(world_width)
				, world_height(world_height)
				, row_length(row_length)
				, column_length(column_length)
			{
				auto rows = static_cast<int>(row_length);
				auto columns = static_cast<int>(column_length);
			
				int cell_width = static_cast<int>(world_width) / rows;
				int cell_height = static_cast<int>(world_height) / columns;

				for (int row = 0; row < rows; row++) {
					for (int col = 0; col < columns; col++)
					{
						cells.push_back(Cell(row * cell_width, col * cell_height, cell_width, cell_height));
					}
				}
			}

			// Returns the number of entities mapped in the spatial partition
			std::size_t size() const noexcept { return entity_lookup_table.size(); }

			// Checks if the spatial partition contains an entity
			bool contains(const rift::Entity& entity) const noexcept {
				return entity_lookup_table.find(entity.id().number()) != entity_lookup_table.end();
			}

			// Return the correct cell for an entity at a given position in space
			template <class T>
			std::size_t cell_for(const Vec2<T>& position) const noexcept {
				auto column = static_cast<std::size_t>(row_length * position.x / world_width);
				auto row = static_cast<std::size_t>(column_length * position.y / world_height);
				auto cell = column_length * column + row;
				return cell_index >= cells.size() ? cells.size() - 1 : cell;
			}

			// Insert an entity into the spatial partitioning using their position in space
			template <class T>
			void insert(const rift::Entity& entity, const Vec2<T>& position) noexcept {
				assert(!contains(entity) && "Cannot insert the same entity twice!");
				auto cell = cell_for(position);
				auto e(entity);
				cells.at(cell).members.insert(entity.id().index(), &e);
				entity_lookup_table.insert(std::make_pair(entity.id().number(), cell));
			}

			// Remove an entity from the spatial partitioning
			void remove(const rift::Entity& entity) noexcept {
				assert(contains(entity) && "Cannot an unmanaged entity!");
				auto cell = entity_lookup_table.at(entity.id().number());
				cells.at(cell).members.remove(entity.id().index());
				entity_lookup_table.erase(entity.id().number());
			}

			// Update the entity's position in the partitioning with the new position
			template <class T>
			void update(const rift::Entity& entity, const Vec2<T>& position) noexcept {
				assert(contains(entity) && "Cannot update the position of an unmanaged entity!");
				auto new_cell = cell_for(position);
				auto old_cell = entity_lookup_table.at(entity.id().number());
				if (new_cell != old_cell) {
					auto e(entity);
					cells.at(old_cell).members.remove(entity.id().index());
					cells.at(new_cell).members.insert(entity.id().index(), &e);
					entity_lookup_table.at(entity.id().number()) = new_cell;
				}
			}

			// Applies f onto the entities that are within the proximity circle
			template <class T>
			void for_each_neighbour(const Vec2<T>& position, T prox_radius, std::function<void(rift::Entity e)> f) {
				Boundary prox_box(static_cast<int>(position.x - prox_radius),
								  static_cast<int>(position.y - prox_radius),
								  static_cast<int>(prox_radius),
								  static_cast<int>(prox_radius));

				for (auto cell : cells) {
					if (prox_box.intersects(cell.boundary)) {
						for (auto entity : cell.members) {
							f(entity);
						}
					}
				}
			}

		private:

			// The dimensions of space being partitioned
			std::size_t world_width, world_height;
			
			// The number of vertical and horizontal cells
			std::size_t row_length, column_length;
			
			// Collection of cells which contain entities
			// Stored in column major order
			std::vector<Cell> cells; 
			
			// Look up table for finding entities
			std::unordered_map<std::uint64_t, std::size_t> entity_lookup_table;

		};

	}
}