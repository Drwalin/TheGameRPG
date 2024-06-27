#include <iostream>
#include <string>

#include "../include/ServerCore.hpp"

void ServerCore::RunMainThreadInteractive()
{
	while (requestStop == false) {
		std::string cmd;
		std::getline(std::cin, cmd);
		commandParser.ParseSingleCommand(cmd);
	}
}
