#pragma once

#include <bitset>

namespace rift {

	namespace config {

		static const std::size_t MAX_COMPONENT_TYPES = 128;

	}
	using ComponentMask = std::bitset<config::MAX_COMPONENT_TYPES>;
}
