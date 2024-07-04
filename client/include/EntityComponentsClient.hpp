#pragma once

#include <vector>

#include "../../common/include/EntityComponents.hpp"

struct ComponentMovementHistory {
	std::vector<ComponentMovementState> states;

	DEFAULT_CONSTRUCTORS_AND_MOVE(ComponentMovementHistory, MV(states));
};
