#pragma once

#include <cstdint>

#include "../../ICon7/include/icon7/Time.hpp"

class TickTimer
{
public:
	inline void Start()
	{
		currentTick = 0;
		startTickCountingTime = icon7::time::GetTemporaryTimestamp();
	}

	inline void Start(int64_t currentTick)
	{
		Start();
		this->currentTick = currentTick;
		startTickCountingTime -= currentTick*1000ll*1000ll;
	}

	[[nodiscard]] inline int64_t GetCurrentTick() const { return currentTick; }

	inline void Update() { currentTick = CalcCurrentTick(); }

	[[nodiscard]] inline int64_t CalcCurrentTick() const
	{
		const auto currentTime = icon7::time::GetTemporaryTimestamp();
		return TicksBetween(startTickCountingTime, currentTime);
	}

	inline static int64_t TicksBetween(int64_t start, int64_t end)
	{
		return (end - start)/1000ll*1000ll;
	}

public:
	int64_t startTickCountingTime;
	int64_t currentTick = 0;
};
