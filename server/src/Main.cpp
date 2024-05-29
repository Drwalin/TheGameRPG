#include "../include/ServerCore.hpp"

int main(int argc, char **argv)
{
	icon7::Initialize();
	LOG_INFO("Main thread started");
	{
		ServerCore serverCore;
		serverCore.BindRpc();
		serverCore.CreateRealm("World1");
		serverCore.CreateRealm("Heaven");
		serverCore.CreateRealm("MiddleEarth");

		serverCore.StartService();
		serverCore.Listen("localhost", 25369, true);
		serverCore.Listen("127.0.0.1", 25369, true);
		serverCore.Listen("::1", 25369, true);
		for (int i = 1; i < argc; ++i) {
			serverCore.Listen(argv[i], 25369, true);
		}

		serverCore.realmManager.RunAsync(1);

		serverCore.RunNetworkLoopAsync();

		serverCore.RunMainThreadInteractive();
	}
	icon7::Deinitialize();
	return 0;
}
