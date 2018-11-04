#pragma once

#include "../system.h"
#include "../component.h"

#ifdef _USE_DOUBLE_PRECISION_FP_
using _FP_PRECISION_TYPE_ = double;
#else
using _FP_PRECISION_TYPE_ = float;
#endif // _USE_DOUBLE_PRECISION_FP_

#include "details/vec2.h"
#include "details/spatial.h"

namespace rift {

	enum class Behaviour {
		NONE = 0x00000,
		SEEK = 0x00001,
		FLEE = 0x00002,
	};

	using Vec2D = details::Vec2<_FP_PRECISION_TYPE_>;
	
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
