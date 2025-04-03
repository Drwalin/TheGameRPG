#include <cmath>

#include <algorithm>

#include "../../ICon7/include/icon7/Time.hpp"
#include "../../ICon7/include/icon7/Debug.hpp"

#include "../include/StatsCollector.hpp"

std::string StatsCollector::StatEntry::ToString() const
{
	std::string ret;

	double seconds =
		icon7::time::DeltaSecBetweenTimestamps(timestampStart, timestampEnd);

	char str[4096];
	snprintf(str, 4095,
			 "stats:"
			 "  count: %8lu"
			 "  min: %8.3f"
			 "  avg: %8.3f"
			 "  max: %8.3f"
			 "      p25: %8.3f"
			 "  p33: %8.3f"
			 "  p50: %8.3f"
			 "  p66: %8.3f"
			 "  p75: %8.3f"
			 "  p90: %8.3f"
			 "  p95: %8.3f"
			 "  p99: %8.3f"
			 "  p996: %8.3f"
			 "  p999: %8.3f"
			 "  stddev: %8.3f"
			 "  persec: %10.5f"
			 "  samplingTime: %8.3f [s]"
			 "  %s  -  %s",
			 count, min, avg, max, p25, p33, p50, p66, p75, p90, p95, p99, p996,
			 p999, stddev, (count * avg) / seconds, seconds,
			 icon7::time::TimestampToString(timestampEnd, 3).c_str(),
			 name.c_str());

	ret = str;

	return ret;
}

StatsCollector::StatsCollector(std::string name)
	: stats(name), prevStats(name), name(name)
{
	lastResetTimestamp = icon7::time::GetTimestamp();
	this->name = name;
}

StatsCollector::~StatsCollector() {}

void StatsCollector::PushValue(double value) { values.push_back(value); }

StatsCollector::StatEntry StatsCollector::CalcStats()
{
	stats.count = values.size();
	if (stats.count == 0) {
		return stats = StatsCollector::StatEntry(name, 0, 0);
	} else if (stats.count == 1) {
		return stats = StatsCollector::StatEntry(name, 1, values[0]);
	}
	stats.timestampStart = lastResetTimestamp;
	stats.timestampEnd = icon7::time::GetTimestamp();

	std::sort(values.begin(), values.end());

	double sum = 0.0;
	for (double v : values) {
		sum += v;
	}
	double avg = sum / stats.count;
	stats.stddev = 0.0;
	for (double v : values) {
		double x = v - avg;
		stats.stddev += x * x;
	}
	stats.stddev = sqrt(stats.stddev / stats.count);

	stats.min = values[0];
	stats.avg = avg;
	stats.max = values[stats.count - 1];

	stats.p25 = values[(stats.count * 25) / 100];
	stats.p33 = values[(stats.count * 33) / 100];
	stats.p50 = values[(stats.count * 50) / 100];
	stats.p66 = values[(stats.count * 66) / 100];
	stats.p75 = values[(stats.count * 75) / 100];
	stats.p90 = values[(stats.count * 90) / 100];
	stats.p95 = values[(stats.count * 95) / 100];
	stats.p99 = values[(stats.count * 99) / 100];
	stats.p996 = values[(stats.count * 996) / 1000];
	stats.p999 = values[(stats.count * 999) / 1000];

	return stats;
}

void StatsCollector::Reset()
{
	prevStats = stats;
	values.clear();
	lastResetTimestamp = icon7::time::GetTimestamp();
}

StatsCollector::StatEntry StatsCollector::CalcAndReset()
{
	CalcStats();
	Reset();
	return stats;
}

uint64_t StatsCollector::GetSamplesCount() const { return values.size(); }

void StatsCollector::SetName(const std::string &name)
{
	this->name = name;
	stats.name = name;
	prevStats.name = name;
}

void StatsCollector::PrintAndResetStatsIfExpired(int64_t milliseconds)
{
	uint64_t t = icon7::time::GetTimestamp();
	if (icon7::time::DeltaNsBetweenTimestamps(lastResetTimestamp, t) >=
		(milliseconds * (1000ll * 1000ll))) {
		CalcAndReset();
		icon7::log::PrintLineSync("%s", stats.ToString().c_str());
	}
}
