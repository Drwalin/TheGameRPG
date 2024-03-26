#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <string>
#include <atomic>
#include <thread>
#include <memory>
#include <mutex>

#include "RealmServer.hpp"

class RealmWorkThreadedManager final
{
public:
	
	RealmWorkThreadedManager();
	~RealmWorkThreadedManager();
	
	bool AddNewRealm(RealmServer *realm);
	void DestroyRealm(std::string realmName);
	
	void RunAsync(int workerThreadsCount);
	
	void RequestStopRunning();
	void WaitStopRunning();
	bool IsRunning();
	
private:
	void SingleRunner();
	
private:
	std::mutex mutex;
	std::queue<RealmServer *> realmsQueue;
	std::unordered_map<std::string, RealmServer *> realms;
	std::vector<std::string> realmNames;
	std::unordered_set<std::string> realmsToDestroy;
	
	std::atomic<bool> requestStopRunning;
	std::atomic<int> runningThreads;
	std::vector<std::thread> threads;
};
