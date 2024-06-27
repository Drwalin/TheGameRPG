#pragma once

#include "EntityComponents.hpp"

struct CharacterSheet {
	template <typename S> S &__ByteStream_op(S &s) { return s; }
};
