#pragma once

#include "../../ICon7/include/icon7/Time.hpp"

#include "Tick.hpp"

class TickTimer
{
public:
	void Start(icon7::time::diff tickDuration);
	void Start(Tick currentTick, icon7::time::diff tickDuration);

	[[nodiscard]] Tick GetCurrentTick() const;
	void Update(icon7::time::diff tickDuration);
	float GetFactorToNextTick(icon7::time::diff tickDuration) const;

	static icon7::time::point GetCurrentTimepoint();

public:
	icon7::time::Point lastTick;
	icon7::time::Point nextTick;
	Tick currentTick = {0};
};
