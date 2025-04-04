#include <chrono>
#include <mutex>

#include "../../ICon7/include/icon7/Debug.hpp"
#include "../../ICon7/include/icon7/Time.hpp"

#include "../include/RealmServer.hpp"
#include "../include/ServerCore.hpp"
#include "../../common/include/StatsCollector.hpp"

#include "../include/RealmWorkThreadedManager.hpp"

RealmWorkThreadedManager::RealmWorkThreadedManager(ServerCore *serverCore)
	: serverCore(serverCore)
{
	runningThreads = 0;
}

RealmWorkThreadedManager::~RealmWorkThreadedManager()
{
	DestroyAllRealmsAndStop();
}

void RealmWorkThreadedManager::DestroyAllRealmsAndStop()
{
	WaitStopRunning();

	std::lock_guard lock(mutex);
	while (realmsQueue.empty() == false) {
		realmsQueue.pop();
	}
	std::unordered_map<std::string, std::shared_ptr<RealmServer>> r = realms;
	for (auto it : r) {
		it.second->DisconnectAllAndDestroy();
	}
	realms.clear();
}

bool RealmWorkThreadedManager::AddNewRealm(std::shared_ptr<RealmServer> realm)
{
	std::lock_guard lock(mutex);
	if (realms.count(realm->realmName) != 0) {
		return false;
	}
	realms[realm->realmName] = realm;
	realmsQueue.push(realm);
	return true;
}

void RealmWorkThreadedManager::DestroyRealm(std::string realmName)
{
	std::shared_ptr<RealmServer> realm = GetRealm(realmName);
	if (realm != nullptr) {
		realm->QueueDestroy();
	}
}

std::shared_ptr<RealmServer>
RealmWorkThreadedManager::GetRealm(const std::string &realmName)
{
	std::lock_guard lock(mutex);
	auto it = realms.find(realmName);
	if (it == realms.end()) {
		return nullptr;
	}
	return it->second;
}

void RealmWorkThreadedManager::RunAsync(int workerThreadsCount)
{
	millisecondsBetweenStatsReport = serverCore->configStorage.GetOrSetInteger(
		"metrics.thread_manager.realm_process_duration.milliseconds_between_"
		"stats_report",
		60000);
	millisecondsBetweenSleepStatsReport =
		serverCore->configStorage.GetOrSetInteger(
			"metrics.thread_manager.sleeping_duration.milliseconds_between_"
			"stats_report",
			300000);
	currentlyRunningRealmInThread.resize(workerThreadsCount);
	timestampOfStartRunningCurrentRealmInThread.resize(workerThreadsCount);

	requestStopRunning = false;
	std::lock_guard lock(mutex);
	for (int i = threads.size(); i < workerThreadsCount; ++i) {
		threads.push_back(std::thread(
			+[](RealmWorkThreadedManager *manager, int id, int count) {
				LOG_INFO("Realms worker thread %i / %i", id + 1, count);
				manager->SingleRunner(id, count);
			},
			this, i, workerThreadsCount));
	}
}

void RealmWorkThreadedManager::RequestStopRunning()
{
	requestStopRunning = true;
}

void RealmWorkThreadedManager::WaitStopRunning()
{
	RequestStopRunning();
	for (std::thread &t : threads) {
		t.join();
	}
	threads.clear();
}

bool RealmWorkThreadedManager::IsRunning() { return runningThreads > 0; }

void RealmWorkThreadedManager::SingleRunner(int threadId,
											int workerThreadsCount)
{
	StatsCollector statsDuration("RealmWOrkerThread_duration_" +
								 std::to_string(threadId) + ",[ms]");
	StatsCollector statsSleep("RealmWorkerThread_sleep_" +
							  std::to_string(threadId) + ",[ms]");
	currentlyRunningRealmInThread[threadId] =
		std::shared_ptr<RealmServer>(nullptr);

	int notBusyCount = 0;
	++runningThreads;
	int sleepMilliseconds = 1;
	uint32_t countBusySinceLastSleep = 0;
	while (true) {
		uint64_t begin = icon7::time::GetTimestamp();
		int64_t realmsCount = 0;

		bool destroyRealm = false;
		std::shared_ptr<RealmServer> realm = nullptr;
		{
			std::lock_guard lock(mutex);
			realmsCount = realms.size();
			if (realmsQueue.empty() == false) {
				realm = realmsQueue.front();
				realmsQueue.pop();
			}
		}

		if (realm.get() == nullptr && requestStopRunning) {
			break;
		} else if (realm.get()) {
			if (realm->IsQueuedToDestroy() || requestStopRunning == true) {
				std::lock_guard lock(mutex);
				realms.erase(realm->realmName);
				destroyRealm = true;
			}
		}

		currentlyRunningRealmInThread[threadId] = realm;
		timestampOfStartRunningCurrentRealmInThread[threadId] =
			icon7::time::GetTimestamp();

		if (destroyRealm) {
			realm->DisconnectAllAndDestroy();
			// delete realm;
			realm = nullptr;
			notBusyCount = 0;
		} else if (realm) {
			if (realm->RunOneEpoch() == false) {
				notBusyCount++;
			} else {
				notBusyCount = 0;
				countBusySinceLastSleep++;
			}

			{
				std::lock_guard lock(mutex);
				realmsQueue.push(realm);
			}

			realm = nullptr;
		} else {
			notBusyCount++;
		}

		currentlyRunningRealmInThread[threadId] =
			std::shared_ptr<RealmServer>(nullptr);

		uint64_t end = icon7::time::GetTimestamp();
		double duration = icon7::time::DeltaMSecBetweenTimestamps(begin, end);
		statsDuration.PushValue(duration);
		statsDuration.PrintAndResetStatsIfExpired(
			millisecondsBetweenStatsReport);

		if (notBusyCount >= realmsCount / workerThreadsCount) {
			if (countBusySinceLastSleep > 5) {
				sleepMilliseconds = std::max(sleepMilliseconds - 1, 1);
			} else if (countBusySinceLastSleep == 0) {
				sleepMilliseconds = std::min(sleepMilliseconds + 1, 35);
			}
			std::this_thread::sleep_for(
				std::chrono::milliseconds(sleepMilliseconds));
			statsSleep.PushValue(sleepMilliseconds);
			notBusyCount = 0;
			countBusySinceLastSleep = 0;
		}

		statsSleep.PrintAndResetStatsIfExpired(
			millisecondsBetweenSleepStatsReport);
	}
	currentlyRunningRealmInThread[threadId] =
		std::shared_ptr<RealmServer>(nullptr);
	if ((--runningThreads) == 0) {
		requestStopRunning = false;
	}
	currentlyRunningRealmInThread[threadId] =
		std::shared_ptr<RealmServer>(nullptr);
}

std::vector<std::shared_ptr<RealmServer>>
RealmWorkThreadedManager::GetAllRealms()
{
	std::vector<std::shared_ptr<RealmServer>> ret;
	{
		std::lock_guard lock(mutex);
		ret.reserve(realms.size());
		for (auto it : realms) {
			ret.push_back(it.second);
		}
	}
	return ret;
}
