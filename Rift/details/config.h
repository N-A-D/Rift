#pragma once

#include <bitset>

namespace rift {
	namespace config {
		const std::size_t MAX_COMPONENT_TYPES = 64;
	}
	using ComponentMask = std::bitset<config::MAX_COMPONENT_TYPES>;
}