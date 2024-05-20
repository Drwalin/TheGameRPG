#include <chrono>
#include <mutex>

#include "../include/RealmServer.hpp"

#include "../include/RealmWorkThreadedManager.hpp"

RealmWorkThreadedManager::RealmWorkThreadedManager() { runningThreads = 0; }

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
	std::unordered_map<std::string, RealmServer *> r = realms;
	for (auto it : r) {
		it.second->DisconnectAllAndDestroy();
		delete it.second;
	}
	realms.clear();
	realmsToDestroy.clear();
}

bool RealmWorkThreadedManager::AddNewRealm(RealmServer *realm)
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
	std::lock_guard lock(mutex);
	if (realms.count(realmName) == 0) {
		return;
	}
	realmsToDestroy.insert(realmName);
}

RealmServer *RealmWorkThreadedManager::GetRealm(const std::string &realmName)
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
	requestStopRunning = false;
	std::lock_guard lock(mutex);
	for (int i = threads.size(); i < workerThreadsCount; ++i) {
		threads.push_back(std::thread(
			+[](RealmWorkThreadedManager *manager) { manager->SingleRunner(); },
			this));
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

void RealmWorkThreadedManager::SingleRunner()
{
	int notBusyCount = 0;
	++runningThreads;
	int sleepMilliseconds = 1;
	uint32_t countBusySinceLastSleep = 0;
	while (requestStopRunning == false) {
		bool destroyRealm = false;
		RealmServer *realm = nullptr;
		{
			std::lock_guard lock(mutex);
			if (realmsQueue.empty() == false) {
				realm = realmsQueue.front();
				realmsQueue.pop();
				if (realmsToDestroy.count(realm->realmName) != 0) {
					realms.erase(realm->realmName);
					realmsToDestroy.erase(realm->realmName);
				}
			}
		}

		if (destroyRealm) {
			realm->DisconnectAllAndDestroy();
			delete realm;
			realm = nullptr;
			notBusyCount = 0;
		} else if (realm) {
			if (realm->OneEpoch() == false) {
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
		if (notBusyCount >= 6) {
			if (countBusySinceLastSleep > 5) {
				sleepMilliseconds = std::max(sleepMilliseconds - 1, 1);
			} else if (countBusySinceLastSleep == 0) {
				sleepMilliseconds = std::min(sleepMilliseconds + 1, 35);
			}
			std::this_thread::sleep_for(
				std::chrono::milliseconds(sleepMilliseconds));
			notBusyCount = 0;
			countBusySinceLastSleep = 0;
		}
	}
	if ((--runningThreads) == 0) {
		requestStopRunning = false;
	}
}
