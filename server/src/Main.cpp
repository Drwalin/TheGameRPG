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
	
	DEBUG("Starting to listen...");
	
	serverCore.StartListening(25369, true);
	
	DEBUG("Running async thread...");
	
	serverCore.RunNetworkLoopSync();
	
// 	DEBUG("Connecting...");
// 	auto _peer = serverCore.host->ConnectPromise("127.0.0.1", 25369);
// 	
// 	DEBUG("Waiting on peer promise");
// 	auto peer = _peer.get();
// 	DEBUG("Connected to: %p", peer.get());
// 	if (peer != nullptr) {
// 		while(peer->IsReadyToUse() == false) {
// 			std::this_thread::sleep_for(std::chrono::milliseconds(10));
// 		}
// 		peer->Disconnect();
// 	}
// 	peer = nullptr;
// 	
// 	while(true) {
// 		std::this_thread::sleep_for(std::chrono::milliseconds(500));
// 	}

	}
	icon7::Deinitialize();
	return 0;
}
