#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>

class RealmServer;

class RealmWorkThreadedManager final
{
public:
	RealmWorkThreadedManager();
	~RealmWorkThreadedManager();
	
	void DestroyAllRealmsAndStop();

	bool AddNewRealm(RealmServer *realm);
	void DestroyRealm(std::string realmName);

	void RunAsync(int workerThreadsCount);

	void RequestStopRunning();
	void WaitStopRunning();
	bool IsRunning();

	RealmServer *GetRealm(const std::string &realmName);

private:
	void SingleRunner();

private:
	std::mutex mutex;
	std::queue<RealmServer *> realmsQueue;
	std::unordered_map<std::string, RealmServer *> realms;
	std::unordered_set<std::string> realmsToDestroy;

	std::atomic<bool> requestStopRunning;
	std::atomic<int> runningThreads;
	std::vector<std::thread> threads;
};
