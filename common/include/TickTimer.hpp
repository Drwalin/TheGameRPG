#pragma once

#include <cstdint>

#include <chrono>

class TickTimer
{
public:
	using steady_clock = std::chrono::steady_clock;
	using milliseconds = std::chrono::milliseconds;
	using time_point = steady_clock::time_point;

	inline void Start()
	{
		currentTick = 0;
		startTickCountingTime = steady_clock::now();
	}

	inline void Start(int64_t currentTick)
	{
		Start();
		this->currentTick = currentTick;
		startTickCountingTime -= milliseconds(currentTick);
	}

	[[nodiscard]] inline int64_t GetCurrentTick() const { return currentTick; }

	inline void Update() { currentTick = CalcCurrentTick(); }

	[[nodiscard]] inline int64_t CalcCurrentTick() const
	{
		const auto currentTime = steady_clock::now();
		return TicksBetween(startTickCountingTime, currentTime);
	}

	inline static int64_t TicksBetween(time_point start, time_point end)
	{
		return std::chrono::duration_cast<milliseconds>(end - start).count();
	}

public:
	time_point startTickCountingTime;
	int64_t currentTick = 0;
};
