#pragma once

#include <glm/glm.hpp>

#include <icon7/Command.hpp>

class CommandTeleportPlayer : public icon7::commands::ExecuteOnHost
{
public:
	CommandTeleportPlayer() = default;
	~CommandTeleportPlayer() = default;

	class ServerCore *serverCore;
	std::string userName;
	std::string realmName;
	glm::vec3 position;

	virtual void Execute() override;
};
