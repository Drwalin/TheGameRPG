#pragma once

#include <cstdint>

enum CollisionFilter : uint32_t {
	FILTER_ALL = 0xFFFFFFFF,
	FILTER_TERRAIN = 1,
	FILTER_CHARACTER = 2,
	FILTER_STATIC_OBJECT = 4,
	FILTER_TRIGGER = 8,
};

