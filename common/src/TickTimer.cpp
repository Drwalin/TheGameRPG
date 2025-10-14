#include "../include/TickTimer.hpp"

void TickTimer::Start(icon7::time::diff tickDuration)
{
	currentTick = {0};
	lastTick = GetCurrentTimepoint();
	nextTick = lastTick + tickDuration;
}

void TickTimer::Start(Tick currentTick, icon7::time::diff tickDuration)
{
	Start(tickDuration);
	this->currentTick = currentTick;
}

[[nodiscard]] Tick TickTimer::GetCurrentTick() const { return currentTick; }

void TickTimer::Update(icon7::time::diff tickDuration)
{
	++currentTick;
	lastTick = nextTick;
	nextTick += tickDuration;
}

icon7::time::point TickTimer::GetCurrentTimepoint()
{
	return icon7::time::GetTemporaryTimestamp();
}

float TickTimer::GetFactorToNextTick(icon7::time::diff tickDuration) const
{
	auto now = GetCurrentTimepoint();
	return (float)((now - lastTick).ns) / (float)(tickDuration.ns);
}
