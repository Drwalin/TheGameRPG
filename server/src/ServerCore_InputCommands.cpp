
#include <algorithm>
#include <iostream>
#include <string>

#include "../include/ServerCore.hpp"

void ServerCore::RunMainThreadInteractive()
{
	while (requestStop == false) {
		std::string cmd;
		std::getline(std::cin, cmd);
		ParseCommand(cmd);
	}
}

void ServerCore::ParseCommand(const std::string &_cmd)
{
	std::string cmd = _cmd;
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), [](auto c){ return std::tolower(c); });
	
	if (cmd == "quit" || cmd == "exit" || cmd == "stop") {
		requestStop = true;
		
		Destroy();
	}
}
