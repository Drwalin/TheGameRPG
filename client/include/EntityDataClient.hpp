#pragma once

#include <vector>

#include "../../common/include/EntityData.hpp"

struct EntityMovementHistory {
	std::vector<EntityMovementState> states;
};
