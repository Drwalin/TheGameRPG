#pragma once

#include "../../ICon7/include/icon7/Time.hpp"

#include "Tick.hpp"

class TickTimer
{
public:
	inline void Start(icon7::time::diff tickDuration)
	{
		currentTick = {0};
		lastTick = GetCurrentTimepoint();
		nextTick = lastTick + tickDuration;
	}

	inline void Start(Tick currentTick, icon7::time::diff tickDuration)
	{
		Start(tickDuration);
		this->currentTick = currentTick;
	}

	[[nodiscard]] inline Tick GetCurrentTick() const { return currentTick; }

	inline void Update(icon7::time::diff tickDuration)
	{
		++currentTick;
		lastTick += tickDuration;
		nextTick += tickDuration;
	}

	inline static icon7::time::point GetCurrentTimepoint()
	{
		return icon7::time::GetTemporaryTimestamp();
	}
	
	inline float GetFactorToNextTick(icon7::time::diff tickDuration)
	{
		auto now = GetCurrentTimepoint();
		return (float)((now - lastTick).ns) / (float)(tickDuration.ns);
	}

public:
	icon7::time::Point lastTick;
	icon7::time::Point nextTick;
	Tick currentTick = {0};
};
