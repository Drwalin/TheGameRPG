#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <string>
#include <atomic>
#include <thread>
#include <memory>
#include <mutex>

#include "../../ICon7/include/icon7/Time.hpp"
#include "../../ICon7/concurrent/thread_safe_value.hpp"

class RealmServer;
class ServerCore;

class RealmWorkThreadedManager final
{
public:
	RealmWorkThreadedManager(ServerCore *serverCore);
	~RealmWorkThreadedManager();

	void DestroyAllRealmsAndStop();

	bool AddNewRealm(std::shared_ptr<RealmServer> realm);
	void DestroyRealm(std::string realmName);

	void RunAsync(int workerThreadsCount);

	void RequestStopRunning();
	void WaitStopRunning();
	bool IsRunning();

	std::shared_ptr<RealmServer> GetRealm(const std::string &realmName);

	std::vector<std::shared_ptr<RealmServer>> GetAllRealms();

private:
	void SingleRunner(int threadId, int workerThreadsCount);

	icon7::time::Diff timeBetweenStatsReport = icon7::time::seconds(60);
	icon7::time::Diff timeBetweenSleepStatsReport = icon7::time::seconds(300);

private:
	std::mutex mutex;
	std::queue<std::shared_ptr<RealmServer>> realmsQueue;
	std::unordered_map<std::string, std::shared_ptr<RealmServer>> realms;

	std::atomic<bool> requestStopRunning;
	std::atomic<int> runningThreads;
	std::vector<std::thread> threads;

	std::vector<concurrent::thread_safe_value<std::shared_ptr<RealmServer>>>
		currentlyRunningRealmInThread;
	std::vector<icon7::time::Timestamp>
		timestampOfStartRunningCurrentRealmInThread;

	ServerCore *serverCore;
};
