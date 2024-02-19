#include <cstdio>

#include <future>
#include <iostream>
#include <chrono>
#include <memory>
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
	
	serverCore.StartListening(25369, true);
	
	serverCore.RunNetworkLoopSync();

	}
	icon7::Deinitialize();
	return 0;
}
