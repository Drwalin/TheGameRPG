#include <thread>

#include "include/FileOperations.hpp"
#include "include/ServerCore.hpp"

int RegisterEntityGameComponents(flecs::world &ecs);

int main(int argc, char **argv)
{
	icon7::Initialize();
	LOG_INFO("Main thread started");
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

		serverCore.RunMainThreadInteractive();
	}
	icon7::Deinitialize();
	return 0;
}
