#include <sstream>
#include <algorithm>

#include <icon7/Debug.hpp>

#include "../include/ServerCore.hpp"
#include "../include/StringUtil.hpp"

#include "../include/CommandParser.hpp"

CommandParser::CommandParser(ServerCore *serverCore) : serverCore(serverCore) {}

CommandParser::~CommandParser() {}

void CommandParser::ParseSingleCommand(const std::string &command)
{
	_InternalParseCommand(command);
}

void CommandParser::ParseCommands(const std::string &commands)
{
	std::istringstream ss(commands);
	ParseCommands(ss);
}

void CommandParser::ParseCommands(std::istream &input)
{
	input >> std::ws;
	std::string line;
	while (input.eof() == false && input.good()) {
		line = "";
		std::getline(input, line);
		input >> std::ws;
		ParseSingleCommand(line);
	}
}

std::string CommandParser::GetFullHelp() const
{
	std::string ret;
	for (const auto &it : commands) {
		if (it.first == it.second->alternatives[0]) {
			if (ret.size() > 0) {
				ret += "\n\n";
			}
			ret += it.second->description;
		}
	}
	return ret;
}

std::string CommandParser::GetCommandsList() const
{
	std::string ret;
	for (const auto &it : commands) {
		if (it.first == it.second->alternatives[0]) {
			ret += "\n  ";
			ret += it.first;
		}
	}
	return ret;
}

void CommandParser::RegisterCustomCommand(
	const std::vector<std::string> &commandNames, std::string description,
	std::function<void(const std::vector<std::string> &args)> function)
{
	std::shared_ptr<CommandStorage> cmdStorage =
		std::make_shared<CommandStorage>();
	cmdStorage->alternatives = commandNames;
	cmdStorage->function = function;

	std::string conflict = "";
	for (int i = 0; i < cmdStorage->alternatives.size(); ++i) {
		std::string &cmd = cmdStorage->alternatives[i];
		std::string oldCmd = cmd;
		for (int j = 1; j <= 1000; ++j) {
			std::transform(cmd.begin(), cmd.end(), cmd.begin(),
						   [](auto c) { return std::tolower(c); });
			auto it = commands.find(cmd);
			if (it != commands.end()) {
				cmd = oldCmd + "_" + std::to_string(j);
				cmdStorage->alternatives[i] = cmd;
				conflict = it->second->alternatives[0];
			} else {
				if (j > 1) {
					LOG_ERROR("There is conflict between `%s` and `%s` of: "
							  "`%s`, replacing with: `%s` for `%s`",
							  conflict.c_str(),
							  cmdStorage->alternatives[0].c_str(),
							  oldCmd.c_str(), cmd.c_str(),
							  cmdStorage->alternatives[0].c_str());
				}
				commands[cmd] = cmdStorage;
				break;
			}
		}
	}

	cmdStorage->description = "";
	for (auto cmd : cmdStorage->alternatives) {
		if (cmdStorage->description.size() > 0) {
			cmdStorage->description += ", ";
		}
		cmdStorage->description += cmd;
	}
	cmdStorage->description +=
		"\n  " + std::replace_all(description, "\n", "\n  ");
}

void CommandParser::_InternalParseCommand(const std::string &command)
{
	auto _args = std::convert_to_args_list({command.data(), command.size()});
	if (_args.size() == 0) {
		return;
	}

	std::vector<std::string> args;
	args.resize(_args.size());
	for (int i = 0; i < args.size(); ++i) {
		args[i] = std::to_string(_args[i]);
		if (i > 0) {
			// TODO: substitute variablies like in bash: ${varname}
			/*
			std::string v = std::to_string(_args[i]);
			int64_t ni = 0;
			double nf = 0;
			if (serverCore->configStorage.GetString(v, &(args[i]))) {
				continue;
			} else if (serverCore->configStorage.GetInteger(v, &ni)) {
				args[i] = std::to_string(ni);
			} else if (serverCore->configStorage.GetFloat(v, &nf)) {
				args[i] = std::to_string(nf);
			} else {
				args[i]
			}
			*/
		}
	}

	std::string cmd = std::to_string(args[0]);
	std::transform(cmd.begin(), cmd.end(), cmd.begin(),
				   [](auto c) { return std::tolower(c); });
	auto it = commands.find(cmd);
	if (it != commands.end()) {
		it->second->function(args);
	} else {
		std::string s;
		for (std::string sv : args) {
			s += "`";
			s += std::to_string(sv);
			s += "`";
		}
		LOG_INFO("Command not found: %s", s.c_str());
	}
}
