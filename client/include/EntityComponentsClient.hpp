#pragma once

#include <vector>

#include "../../ICon7/include/icon7/Time.hpp"

#include "../../common/include/EntityComponents.hpp"

struct ComponentMovementHistory {
	std::vector<ComponentMovementState> states;

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentMovementHistory, MV(states));
};

struct ComponentLastAuthoritativeStateUpdateTime {
	icon7::time::Point timepoint;
	int64_t tick;
	int64_t tickInState;

	ComponentLastAuthoritativeStateUpdateTime(
		icon7::time::Point tp, int64_t tick,
		int64_t tickInState)
		: timepoint(tp), tick(tick), tickInState(tickInState)
	{
	}

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentLastAuthoritativeStateUpdateTime,
								  {MV(timepoint) MV(tick) MV(tickInState)});
};
