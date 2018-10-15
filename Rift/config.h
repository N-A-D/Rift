#pragma once
#ifndef _RIFT_CONFIG_
#define _RIFT_CONFIG_
#include <bitset>

namespace rift {
	// Defines rift's configuration information
	namespace config {
		// If more component types are needed modify this value
		const std::size_t MAX_COMPONENT_TYPES = 64;
	}
	using ComponentMask = std::bitset<config::MAX_COMPONENT_TYPES>;
}
#endif // !_RIFT_CONFIG_