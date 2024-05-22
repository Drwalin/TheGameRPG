#include "../include/ServerCore.hpp"
#include "../include/DBWorker.hpp"

int main(int argc, char **argv)
{
	icon7::Initialize();
	LOG_INFO("Main thread started");
	{
		DBWorker::GetSingleton()->Init("database.db");
		DBWorker::GetSingleton()->RunAsync();
		
		ServerCore serverCore;
		serverCore.BindRpc();
		serverCore.CreateRealm("World1");
		serverCore.CreateRealm("Middle Earth");
		serverCore.CreateRealm("Heaven");

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
	DBWorker::GetSingleton()->Destroy();
	icon7::Deinitialize();
	return 0;
}
