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
				NONE = 1,
				SEEK = 2,
				FLEE = 4
			};

		}
	}
}

#endif // !_RIFT_AUTONOMOUS_AGENT_CONFIG_
