#include <thread>
#include <algorithm>

#include "../ICon7/include/icon7/Debug.hpp"
#include "../ICon7/include/icon7/Host.hpp"

#include "include/FileOperations.hpp"
#include "include/ServerCore.hpp"

int RegisterEntityGameComponents(flecs::world &ecs);

void InitFlecs()
{
	flecs::world initializer;
	RegisterEntityGameComponents(initializer);
}

void LoadDefaultConfig(ServerCore *serverCore, int argc, char **argv)
{
	const std::string prefix = "source ";
	const char *paths[4] = {argc >= 2 ? argv[1] : nullptr, "default.cfg",
							"../default.cfg", "../game_assets/default.cfg"};
	for (auto path : paths) {
		std::string command = prefix + "'" + path + "'";
		if (FileOperations::FileExists(path)) {
			serverCore->commandParser.ParseSingleCommand(command);
			break;
		}
	}
}

int main(int argc, char **argv)
{
	LOG_INFO("Main thread started");
	srand(time(NULL));

	icon7::Initialize();
	InitFlecs();
	ServerCore serverCore;
	serverCore.BindRpc();

	serverCore.StartService();
	serverCore.commandParser.InitializeCommands();

	LoadDefaultConfig(&serverCore, argc, argv);

	serverCore.realmManager.RunAsync(
		std::clamp<int64_t>(serverCore.configStorage.GetOrSetInteger(
								"config.realm_worker_manager.threads_count", 1),
							1, std::thread::hardware_concurrency()));

	serverCore.RunNetworkLoopAsync();

	serverCore.RunMainThreadInteractive();

	icon7::Deinitialize();
	return 0;
}
