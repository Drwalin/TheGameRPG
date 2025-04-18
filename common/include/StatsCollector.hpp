#pragma once
#include <cstdint>

#include <vector>
#include <string>

#include "../../ICon7/include/icon7/Time.hpp"

class StatsCollector
{
public:
	struct StatEntry {
		inline StatEntry(const std::string &name, uint64_t count = 0,
						 double v = -1.0)
			: name(name), count(count), min(v), avg(count), max(v), p25(v),
			  p33(v), p50(v), p66(v), p75(v), p90(v), p99(v), p996(v), p999(v),
			  stddev(0.0)
		{
		}

		StatEntry(const StatEntry &) = default;
		StatEntry &operator=(const StatEntry &) = default;

		std::string name;

		uint64_t count = 0;

		double min = -1.0;
		double avg = -1.0;
		double max = -1.0;

		double p25 = -1.0;
		double p33 = -1.0;
		double p50 = -1.0;
		double p66 = -1.0;
		double p75 = -1.0;
		double p90 = -1.0;
		double p95 = -1.0;
		double p99 = -1.0;
		double p996 = -1.0;
		double p999 = -1.0;

		double stddev = -1.0;

		icon7::time::Timestamp timestampStart;
		icon7::time::Timestamp timestampEnd;

		std::string ToString() const;
	};

	StatsCollector(std::string name);
	StatsCollector(const StatsCollector &other) = default;
	StatsCollector &operator=(const StatsCollector &other) = default;
	~StatsCollector();

	void PushValue(double value);
	StatEntry CalcStats();
	void Reset();
	uint64_t GetSamplesCount() const;
	StatEntry CalcAndReset();

	void SetName(const std::string &name);

	void PrintAndResetStatsIfExpired(icon7::time::Diff dt);

public:
	StatEntry stats;
	StatEntry prevStats;

private:
	std::string name;
	std::vector<double> values;
	icon7::time::Timestamp lastResetTimestamp;
};
