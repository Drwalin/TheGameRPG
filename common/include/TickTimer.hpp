#pragma once

#include <cstdint>

#include "../../ICon7/include/icon7/Time.hpp"

class TickTimer
{
public:
	inline void Start()
	{
		currentTick = 0;
		startTickCountingTimePoint = icon7::time::GetTemporaryTimestamp();
	}

	inline void Start(int64_t currentTick)
	{
		Start();
		this->currentTick = currentTick;
		startTickCountingTimePoint -= icon7::time::milliseconds(currentTick);
	}

	[[nodiscard]] inline int64_t GetCurrentTick() const { return currentTick; }

	inline void Update() { currentTick = CalcCurrentTick(); }

	[[nodiscard]] inline int64_t CalcCurrentTick() const
	{
		const auto currentTime = icon7::time::GetTemporaryTimestamp();
		return TicksBetween(startTickCountingTimePoint, currentTime);
	}

	inline static int64_t TicksBetween(icon7::time::Point start,
									   icon7::time::Point end)
	{
		return (end - start).ns / (1'000'000ll);
	}

public:
	icon7::time::Point startTickCountingTimePoint;
	int64_t currentTick = 0;
};
