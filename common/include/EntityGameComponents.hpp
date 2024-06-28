#pragma once

#include "EntityComponents.hpp"

struct CharacterSheet {
	float useRange = 4.0f;

	template <typename S> S &__ByteStream_op(S &s)
	{
		s.op(useRange);
		return s;
	}
};
