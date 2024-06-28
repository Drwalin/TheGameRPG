#include "include/FileOperations.hpp"
#include "include/ServerCore.hpp"

int main(int argc, char **argv)
{
	icon7::Initialize();
	LOG_INFO("Main thread started");
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
					printf("Exists: %s\n", path.c_str());
					serverCore.commandParser.ParseSingleCommand(prefix + path);
					break;
				}
			}
		}

		serverCore.realmManager.RunAsync(1);

		serverCore.RunNetworkLoopAsync();

		serverCore.RunMainThreadInteractive();
	}
	icon7::Deinitialize();
	return 0;
}
