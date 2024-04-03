#include <chrono>
#include <thread>

#include "../include/ServerCore.hpp"

int main()
{
	icon7::Initialize();
	{
		ServerCore serverCore;
		serverCore.BindRpc();
		serverCore.CreateRealm("World1");
		serverCore.CreateRealm("Middle Earth");
		serverCore.CreateRealm("Heaven");
		
		DEBUG("Starting to listen...");
		
		serverCore.StartListening(25369, true);
		
		DEBUG("Running async thread...");
		
		serverCore.realmManager.RunAsync(1);
		
		serverCore.host->RunAsync();
		std::this_thread::sleep_for(std::chrono::seconds(60));
		while (serverCore.host->IsRunningAsync()) {
			std::this_thread::sleep_for(std::chrono::seconds(16));
		}
	}
	icon7::Deinitialize();
	return 0;
}
