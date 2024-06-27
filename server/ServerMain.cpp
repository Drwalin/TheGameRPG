#include "../include/ServerCore.hpp"

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
			serverCore.commandParser.ParseSingleCommand("source default.cfg");
		}

		serverCore.realmManager.RunAsync(1);

		serverCore.RunNetworkLoopAsync();

		serverCore.RunMainThreadInteractive();
	}
	icon7::Deinitialize();
	return 0;
}
