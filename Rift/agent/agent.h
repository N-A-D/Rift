#pragma once

#include "../system.h"
#include "../component.h"
#include "../event.h"

#ifdef _AGENTS_USE_DOUBLE_PRECISION_FP_
using _AGENT_FP_PRECISION_TYPE_ = double;
#else
using _AGENT_FP_PRECISION_TYPE_ = float;
#endif // AGENTS_USE_DOUBLE_PRECISION_FP_

#include "details/vec2.h"
#include "details/spatial.h"

namespace rift {
	namespace agent {
		// Contains the new position of an entity
		struct MovementEvent : public Event<MovementEvent> {
			MovementEvent() : x(0), y(0) {}
			MovementEvent(rift::Entity entity, _AGENT_FP_PRECISION_TYPE_ x, _AGENT_FP_PRECISION_TYPE_ y)
				: entity(entity), x(x), y(y) {}
			rift::Entity entity;
			_AGENT_FP_PRECISION_TYPE_ x, y;
		};

		enum class Behaviour {
			NONE = 0x00000,
			SEEK = 0x00001,
			FLEE = 0x00002,
			HIDE = 0x00004,
			EVADE = 0x00008,
			ARRIVE = 0x00010,
			WANDER = 0x00020,
			PURSUIT = 0x00040,
			INTERPOSE = 0x00080,
			COHESION = 0x00100,
			SEPARATION = 0x00200,
			ALIGNMENT = 0x00400,
		};

		using Vec2D = details::Vec2<_AGENT_FP_PRECISION_TYPE_>;

		struct AutonomousBody : public Component<AutonomousBody> {

			Vec2D pos;
			Vec2D vel;
			Vec2D dir;

		};

		class AutonomousBodySystem : public System<AutonomousBodySystem> {
		public:
		private:
			details::CellSpacePartition cell_space;
		};
	}
}
