#include <thread>
#include <algorithm>

#include "../../../thirdparty/bullet/src/BulletCollision/BroadphaseCollision/btSimpleBroadphase.h"

#include "../ICon7/include/icon7/Debug.hpp"
#include "../ICon7/include/icon7/Host.hpp"
#include "../ICon7/include/icon7/MemoryPool.hpp"

#include "include/FileOperations.hpp"
#include "include/ServerCore.hpp"

int RegisterEntityGameComponents(flecs::world &ecs);

std::atomic<size_t> allocatedBytes = 0;
std::atomic<size_t> allocatedBlocks = 0;
std::atomic<size_t> totalMallocs = 0;

void *Allocate(size_t size)
{
	if (size != 0) {
		totalMallocs++;
		allocatedBlocks++;
		allocatedBytes += size;
		auto o = icon7::MemoryPool::Allocate(size + 16);
		((size_t *)o.object)[0] = size;
		((size_t *)o.object)[1] = o.capacity;
		char *ptr = (char *)o.object;
		return ptr + 16;
	}
	exit(13);
	return nullptr;
}

void Free(void *ptr)
{
	if (ptr) {
		allocatedBlocks--;
		size_t *g = (size_t *)(((char *)ptr) - 16);
		size_t size = g[0];
		size_t capacity = g[1];
		allocatedBytes -= size;
		icon7::MemoryPool::Release(g, capacity);
	}
}

int main(int argc, char **argv)
{
	btAlignedAllocSetCustom(Allocate, Free);
	LOG_INFO("Main thread started");
	srand(time(NULL));
	icon7::Initialize();
	{
		flecs::world initializer;
		RegisterEntityGameComponents(initializer);
	}
	{
		ServerCore serverCore;
		serverCore.BindRpc();

		serverCore.StartService();
		serverCore.commandParser.InitializeCommands();

		if (argc >= 2) {
			serverCore.commandParser.ParseSingleCommand(
				std::string("source '") + argv[1] + "'");
		} else {
			const std::string prefix = "source ";
			std::vector<std::string> files = {"default.cfg", "../default.cfg",
											  "../game_assets/default.cfg"};
			for (std::string path : files) {
				if (FileOperations::FileExists(path)) {
					serverCore.commandParser.ParseSingleCommand(prefix + path);
					break;
				}
			}
		}

		serverCore.realmManager.RunAsync(std::clamp<int64_t>(
			serverCore.configStorage.GetOrSetInteger(
				"config.realm_worker_manager.threads_count", 1),
			1, std::thread::hardware_concurrency()));

		serverCore.RunNetworkLoopAsync();

		/*
		std::thread([]() {
			while (true) {
				LOG_INFO("Bullet memory usage: %.2f MiB (usage test overhead: "
						 "%lu -> %.2f MiB), total mallocs count: %lu",
						 allocatedBytes / (1024.0f * 1024.0f),
						 allocatedBlocks.load(),
						 (allocatedBlocks * 16) / (1024.0f * 1024.0f),
						 totalMallocs.load());
				std::this_thread::sleep_for(std::chrono::seconds(10));
			}
		}).detach();
		*/

		serverCore.RunMainThreadInteractive();
	}
	icon7::Deinitialize();
	return 0;
}
