#pragma once

#include <vector>

#include "../../ICon7/include/icon7/Time.hpp"

#include "../../common/include/Tick.hpp"
#include "../../common/include/EntityComponents.hpp"

struct ComponentMovementHistory {
	struct TimedState {
		ComponentMovementState state__;
		Tick tick;
	};
	std::vector<TimedState> states;

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentMovementHistory, MV(states));
};

struct ComponentLastAuthoritativeStateUpdateTime {
	icon7::time::Point timepoint;
	Tick tick;
	Tick tickInState;

	ComponentLastAuthoritativeStateUpdateTime(icon7::time::Point tp,
											  Tick tick, Tick tickInState)
		: timepoint(tp), tick(tick), tickInState(tickInState)
	{
	}

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentLastAuthoritativeStateUpdateTime,
								  {MV(timepoint) MV(tick) MV(tickInState)});
};
