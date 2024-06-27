#include <fstream>

#include "../include/ServerCore.hpp"
#include "../include/StringUtil.hpp"
#include "../include/SharedObject.hpp"

#include "../include/CommandParser.hpp"

CommandParser::CommandParser(ServerCore *serverCore) : serverCore(serverCore) {}

CommandParser::~CommandParser() {}

void CommandParser::InitializeCommands()
{
	RegisterCustomCommand({"quit", "exit", "stop", "q", ".q"}, R"(Exits game.)",
						  [this](const std::vector<std::string_view> &args) {
							  serverCore->requestStop = true;
							  serverCore->Destroy();
						  });

	RegisterCustomCommand(
		{"help"}, R"(Shows help for commands)",
		[this](const std::vector<std::string_view> &args) {
			std::string helpString;
			if (args.size() > 1) {
				for (int i = 1; i < args.size(); ++i) {
					auto it = commands.find(std::to_string(args[i]));
					if (it == commands.end()) {
						LOG_INFO("No command `%s` exists",
								 std::to_string(args[i]).c_str());
					} else {
						if (helpString.size() > 0) {
							helpString += "\n\n";
						}
						helpString += it->second->description;
					}
				}
			} else {
				helpString = GetFullHelp();
			}
			printf("%s\n", helpString.c_str());
			fflush(stdout);
		});

	RegisterCustomCommand({"list_commands", "list"}, R"(Lists available commands.)",
						  [this](const std::vector<std::string_view> &args) {
							  printf("%s\n", GetCommandsList().c_str());
							  fflush(stdout);
						  });

	RegisterCustomCommand({"source", "run"},
						  R"(Executes contents of given path
arguments:
	- file path with script)",
						  [this](const std::vector<std::string_view> &args) {
							  std::ifstream file(std::to_string(args[1]));
							  ParseCommands(file);
						  });

	RegisterCustomCommand(
		{"load_components"},
		R"(Loads shared object and execute symbols given symbols
arguments:
	- shared object file path
  ... funtion symbols to execute)",
		[this](const std::vector<std::string_view> &args) {
			if (args.size() < 3) {
				LOG_ERROR("Not enaugh arguments to load_components command");
				return;
			}
			std::shared_ptr<SharedObject> so = std::make_shared<SharedObject>();
			if (so->Open(std::to_string(args[1]))) {
				for (int i = 2; i < args.size(); ++i) {
					auto func = so->GetSymbol<void (*)(
						ServerCore *, std::shared_ptr<SharedObject>)>(
						std::to_string(args[i]));
					if (func != nullptr) {
						func(serverCore, so);
					} else {
						LOG_ERROR("Failed to load function `%s` from shared "
								  "object `%s`",
								  std::to_string(args[i]).c_str(),
								  std::to_string(args[1]).c_str());
					}
				}
			} else {
				LOG_ERROR("Failed to load shared object `%s`",
						  std::to_string(args[1]).c_str());
			}
		});

	RegisterCustomCommand({"load_map"},
						  R"(Load map
arguments:
	- realm name)",
						  [this](const std::vector<std::string_view> &args) {
							  for (int i = 1; i < args.size(); ++i) {
								  serverCore->CreateRealm(
									  std::to_string(args[i]));
							  }
						  });

	RegisterCustomCommand(
		{"listen"},
		R"(Listen on interface and port
arguments:
	- listening address
	- listening port
	- ip protocol: 'ipv4' or 'ipv6')",
		[this](const std::vector<std::string_view> &args) {
			if (args.size() != 4) {
				LOG_ERROR("Invalid number of arguments to listen command");
			}
			std::string address = std::to_string(args[1]);
			uint16_t port = stoi(std::to_string(args[2]));
			std::string proto = std::to_string(args[3]);
			bool ipv4 = true;
			if (proto == "ipv6") {
				ipv4 = false;
			}
			serverCore->Listen(address, port, ipv4);
		});
}

void CommandParser::ParseSingleCommand(const std::string &command)
{
	_InternalParseCommand(command);
}

void CommandParser::ParseCommands(const std::string &commands)
{
	std::stringstream ss(commands, std::ios::out);
	ParseCommands(ss);
}

void CommandParser::ParseCommands(std::istream &input)
{
	input >> std::ws;
	std::string line;
	while (input.eof() == false) {
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
			if (ret.size() > 0) {
				ret += "\n";
			}
			ret += it.first;
		}
	}
	return ret;
}

void CommandParser::RegisterCustomCommand(
	const std::vector<std::string> &commandNames, std::string description,
	std::function<void(const std::vector<std::string_view> &args)> function)
{
	std::shared_ptr<CommandStorage> cmdStorage =
		std::make_shared<CommandStorage>();
	cmdStorage->alternatives = commandNames;
	std::string desc;
	cmdStorage->function = function;
	for (auto cmd : cmdStorage->alternatives) {
		cmdStorage->description += cmd + ", ";
		std::transform(cmd.begin(), cmd.end(), cmd.begin(),
					   [](auto c) { return std::tolower(c); });
		commands[cmd] = cmdStorage;
	}
	cmdStorage->description += "\n" + description;
}

void CommandParser::_InternalParseCommand(const std::string &command)
{
	auto args = ConvertToArgsList({command.data(), command.size()});
	if (args.size() == 0) {
		return;
	}
	std::string cmd = std::to_string(args[0]);
	std::transform(cmd.begin(), cmd.end(), cmd.begin(),
				   [](auto c) { return std::tolower(c); });
	auto it = commands.find(cmd);
	if (it != commands.end()) {
		it->second->function(args);
	} else {
		std::string s;
		for (std::string_view sv : args) {
			s += "`";
			s += std::to_string(sv);
			s += "`";
		}
		LOG_INFO("Command not found: %s", s.c_str());
	}
}
