#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <memory>

class RealmServer;

class RealmWorkThreadedManager final
{
public:
	RealmWorkThreadedManager();
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
	void SingleRunner();

private:
	std::mutex mutex;
	std::queue<std::shared_ptr<RealmServer>> realmsQueue;
	std::unordered_map<std::string, std::shared_ptr<RealmServer>> realms;

	std::atomic<bool> requestStopRunning;
	std::atomic<int> runningThreads;
	std::vector<std::thread> threads;
};
