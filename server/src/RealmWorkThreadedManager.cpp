#include "../include/RealmWorkThreadedManager.hpp"
#include <chrono>
#include <mutex>

RealmWorkThreadedManager::RealmWorkThreadedManager()
{
	runningThreads = 0;
}

RealmWorkThreadedManager::~RealmWorkThreadedManager()
{
	WaitStopRunning();
	for (auto it : realms) {
		delete it.second;
	}
}

bool RealmWorkThreadedManager::AddNewRealm(RealmServer *realm)
{
	std::lock_guard lock(mutex);
	if (realms.count(realm->realmName) != 0) {
		return false;
	}
	realms[realm->realmName] = realm;
	realmNames.push_back(realm->realmName);
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

void RealmWorkThreadedManager::RunAsync(int workerThreadsCount)
{
	requestStopRunning = false;
	std::lock_guard lock(mutex);
	for (int i=threads.size(); i<workerThreadsCount; ++i) {
		threads.push_back(std::thread(
					+[](RealmWorkThreadedManager *manager) {
						manager->SingleRunner();
					}, this
					));
	}
}

void RealmWorkThreadedManager::RequestStopRunning()
{
	requestStopRunning = true;
}

void RealmWorkThreadedManager::WaitStopRunning()
{
	RequestStopRunning();
	while(runningThreads > 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(4));
	}
}

bool RealmWorkThreadedManager::IsRunning()
{
	return runningThreads > 0;
}

void RealmWorkThreadedManager::SingleRunner()
{
	int notBusyCount = 0;
	++runningThreads;
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
					realmNames.erase(std::find(realmNames.begin(), realmNames.end(), realm->realmName));
					realmsToDestroy.erase(realm->realmName);
				}
			}
		}
		if (destroyRealm) {
			delete realm;
			realm = nullptr;
			notBusyCount = 0;
		} else if (realm) {
			if (realm->OneEpoch() == false) {
				notBusyCount++;
			} else {
				notBusyCount = 0;
			}
		}
		if (notBusyCount == 10) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
	--runningThreads;
	if (runningThreads == 0) {
		requestStopRunning = false;
	}
}