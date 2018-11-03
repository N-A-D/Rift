#pragma once
#ifndef _RIFT_AUTONOMOUS_AGENT_COMPONENT_
#define _RIFT_AUTONOMOUS_AGENT_COMPONENT_

#include "aa_config.h"
#include "../core/component.h"

namespace rift {

	namespace agent {

		struct AAgent : public Component<AAgent> {

			config::Vec2D pos;
			config::Vec2D vel;

		};

	}

}

#endif // !_RIFT_AUTONOMOUS_AGENT_COMPONENT_
