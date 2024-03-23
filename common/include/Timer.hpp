#pragma once

#include <cinttypes>

#include <chrono>

#include <icon7/Host.hpp>

class Timer
{
public:
	inline void Start()
	{
		currentTick = 0;
		startTickCountingTime = std::chrono::steady_clock::now();
		lastTickCountingTime = startTickCountingTime;
		DEBUG("Init old timer !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	}

	inline void Start(uint64_t currentTick)
	{
		Start();
		this->currentTick = currentTick;
		startTickCountingTime -= std::chrono::milliseconds(currentTick);
		DEBUG("Init new timer !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		DEBUG(" received current %li VVV calc current %li", currentTick, CalcCurrentTick());
	}

	inline uint64_t CalcCurrentTick() const
	{
		const auto currentTime = std::chrono::steady_clock::now();
		return TicksBetween(startTickCountingTime, currentTime);
	}

	inline static uint64_t
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
	uint64_t Update(uint64_t maxDeltaTicks, uint64_t *deltaTicks,
					uint64_t *trueCurrentTick)
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
	uint64_t currentTick;
};
