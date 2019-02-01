#pragma once

#include <bitset>

namespace rift {

	static const std::size_t MAX_COMPONENTS = 128;

	// Component bit mask
	using ComponentMask = std::bitset<MAX_COMPONENTS>;

} // namespace rift
