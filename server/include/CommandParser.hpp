#pragma once

#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <functional>

#include "../include/StringUtil.hpp"

class ServerCore;

class CommandParser
{
public:
	CommandParser(ServerCore *serverCore);
	~CommandParser();

	void InitializeCommands();

	void ParseSingleCommand(const std::string &command);
	void ParseCommands(const std::string &commands);
	void ParseCommands(std::istream &input);

	void RegisterCustomCommand(
		const std::vector<std::string> &commandNames, std::string description,
		std::function<void(const std::vector<std::string_view> &args)>
			function);

	void _InternalParseCommand(const std::string &command);
	
	std::string GetFullHelp() const;
	std::string GetCommandsList() const;

public:
	ServerCore *serverCore;

private:
	struct CommandStorage
	{
		std::function<void(const std::vector<std::string_view> &args)> function;
		std::vector<std::string> alternatives;
		std::string description;
	};
	
	std::unordered_map<
		std::string,
		std::shared_ptr<CommandStorage>
			>
		commands;
};
