#pragma once

#include <vector>

#include "../../common/include/EntityComponents.hpp"

struct ComponentMovementHistory {
	std::vector<ComponentMovementState> states;

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentMovementHistory, MV(states));
};

struct ComponentLastAuthoritativeStateUpdateTime {
	int64_t timepoint;
	int64_t tick;
	int64_t tickInState;

	ComponentLastAuthoritativeStateUpdateTime(
		int64_t tp, int64_t tick,
		int64_t tickInState)
		: timepoint(tp), tick(tick), tickInState(tickInState)
	{
	}

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentLastAuthoritativeStateUpdateTime,
								  {MV(timepoint) MV(tick) MV(tickInState)});
};
