#pragma once
#ifndef _RIFT_AUTONOMOUS_AGENT_CONFIG_
#define _RIFT_AUTONOMOUS_AGENT_CONFIG_

#include "aa_utils.h"

namespace rift {
	namespace agent {
		namespace config {

			using SINGLE_PRECISION = float;
			using DOUBLE_PRECISION = double;

			using Vec2D = impl::Vec2<SINGLE_PRECISION>; // If double precision is needed us DOUBLE_PRECISION

			// Different behavior types
			enum class Behavior {
				NONE       = 0x00000,
				SEEK       = 0x00002,
				FLEE       = 0x00004,
				EVADE      = 0x00008,
				ARRIVE     = 0x00010,
				WANDER     = 0x00020,
				PURSUIT    = 0x00040,
				COHESION   = 0x00080,
				ALIGNMENT  = 0x00100,
				SEPARATION = 0x00200
			};

		}
	}
}

#endif // !_RIFT_AUTONOMOUS_AGENT_CONFIG_
