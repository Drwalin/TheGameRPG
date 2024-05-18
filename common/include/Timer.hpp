#pragma once

#include <cstdint>

#include <chrono>

class Timer
{
public:
	inline void Start()
	{
		currentTick = 0;
		startTickCountingTime = std::chrono::steady_clock::now();
		lastTickCountingTime = startTickCountingTime;
	}

	inline void Start(int64_t currentTick)
	{
		Start();
		this->currentTick = currentTick;
		startTickCountingTime -= std::chrono::milliseconds(currentTick);
	}

	inline int64_t CalcCurrentTick() const
	{
		const auto currentTime = std::chrono::steady_clock::now();
		return TicksBetween(startTickCountingTime, currentTime);
	}

	inline static int64_t
	TicksBetween(std::chrono::steady_clock::time_point start,
				 std::chrono::steady_clock::time_point end)
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(end -
																	 start)
			.count();
	}

	/*
	 * return old value of currentTime if delta time is not sufficient
	 * otherwise return true current tick
	 */
	int64_t Update(int64_t maxDeltaTicks, int64_t *deltaTicks,
				   int64_t *trueCurrentTick)
	{
		const auto currentTime = std::chrono::steady_clock::now();
		*deltaTicks = TicksBetween(lastTickCountingTime, currentTime);
		if (trueCurrentTick) {
			*trueCurrentTick = TicksBetween(startTickCountingTime, currentTime);
		}

		if (*deltaTicks >= maxDeltaTicks) {
			currentTick = TicksBetween(startTickCountingTime, currentTime);
			lastTickCountingTime = currentTime;
		}
		return currentTick;
	}

public:
	std::chrono::steady_clock::time_point startTickCountingTime;
	std::chrono::steady_clock::time_point lastTickCountingTime;
	int64_t currentTick;
};
