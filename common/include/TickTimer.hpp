#pragma once

#include <cstdint>

#include "../../ICon7/include/icon7/Time.hpp"

class TickTimer
{
public:
	inline void Start(icon7::time::diff tickDuration)
	{
		currentTick = 0;
		lastTick = GetCurrentTimepoint();
		nextTick = lastTick + tickDuration;
	}

	inline void Start(int64_t currentTick, icon7::time::diff tickDuration)
	{
		Start(tickDuration);
		this->currentTick = currentTick;
	}

	[[nodiscard]] inline int64_t GetCurrentTick() const { return currentTick; }

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

	/*
	[[nodiscard]] inline int64_t CalcCurrentTick() const
	{
		const auto currentTime = icon7::time::GetTemporaryTimestamp();
		return TicksBetween(startTickCountingTimePoint, currentTime);
	}

	inline static int64_t TicksBetween(icon7::time::Point start,
									   icon7::time::Point end)
	{
		return (end - start).ns / (50'000'000ll);
	}
	*/

public:
	icon7::time::Point lastTick;
	icon7::time::Point nextTick;
	int64_t currentTick = 0;
};
