#include <fstream>

#include "../include/ServerCore.hpp"
#include "../include/ConfigStorage.hpp"
#include "../include/SharedObject.hpp"

#include "../include/CommandParser.hpp"

void CommandParser::InitializeCommands()
{
	serverCore->configStorage.InitialiseCommands(this);

	RegisterCustomCommand({"quit", "exit", "stop", "q", ".q"}, "Exits game.",
						  [this](const std::vector<std::string> &args) {
							  serverCore->requestStop = true;
							  serverCore->Destroy();
						  });

	RegisterCustomCommand(
		{"help"}, "Shows help for commands",
		[this](const std::vector<std::string> &args) {
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

	RegisterCustomCommand(
		{"list_commands", "list"}, "Lists available commands.",
		[this](const std::vector<std::string> &args) {
			std::string msg = GetCommandsList();
			printf("Available commands:\n  %s\n", GetCommandsList().c_str());
			fflush(stdout);
		});

	RegisterCustomCommand({"source", "run"},
						  "Executes contents of given path\n"
						  "arguments:\n"
						  "    - file path with script)",
						  [this](const std::vector<std::string> &args) {
							  std::ifstream file(std::to_string(args[1]));
							  ParseCommands(file);
						  });

	RegisterCustomCommand(
		{"load_symbols", "dlopen", "dlsym"},
		"Loads shared object and execute symbols given symbols\n"
		"arguments:\n"
		"    - shared object file path\n"
		"  ... funtion symbols to execute",
		[this](const std::vector<std::string> &args) {
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
						  "Load map\n"
						  "arguments:\n"
						  "    - realm name",
						  [this](const std::vector<std::string> &args) {
							  for (int i = 1; i < args.size(); ++i) {
								  serverCore->CreateRealm(
									  std::to_string(args[i]));
							  }
						  });

	RegisterCustomCommand(
		{"listen"},
		"Listen on interface and port\n"
		"arguments:\n"
		"    - listening address\n"
		"    - listening port\n"
		"    - ip protocol: 'ipv4' or 'ipv6'",
		[this](const std::vector<std::string> &args) {
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

	RegisterCustomCommand({"list_realms"}, "Lists all loaded realm names",
						  [this](const std::vector<std::string> &args) {
							  auto realms =
								  serverCore->realmManager.GetAllRealms();
							  std::string msg;
							  for (auto r : realms) {
								  msg += "\n  ";
								  msg += r->realmName;
							  }
							  printf("Loaded realms:%s\n", msg.c_str());
						  });
}
