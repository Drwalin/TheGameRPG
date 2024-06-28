#include <mutex>
#include <sstream>

#include <icon7/Debug.hpp>

#include "../include/CommandParser.hpp"
#include "../include/ServerCore.hpp"

#include "../include/ConfigStorage.hpp"

ConfigStorage::ConfigStorage(class ServerCore *serverCore)
	: serverCore(serverCore)
{
}

ConfigStorage::~ConfigStorage()
{
	std::string configSaveFileName;
	if (GetString("config.store_config.file_path", &configSaveFileName)) {
		FILE *file = fopen(configSaveFileName.c_str(), "wb");
		if (file) {
			std::string str = AllConfigsCommand();
			fprintf(file, "%s\n", str.c_str());
			fclose(file);
		} else {
			LOG_ERROR("Failed to open file `%s` for writing",
					  configSaveFileName.c_str());
		}
	}
}

template <typename T>
static bool GetOrDefault(const std::unordered_map<std::string, T> &map,
						 const std::string &key, T *ret)
{
	auto it = map.find(key);
	if (it == map.end()) {
		return false;
	}
	*ret = it->second;
	return true;
}

bool ConfigStorage::GetBool(const std::string &key, bool *ret)
{
	std::shared_lock lock(ms);
	return GetOrDefault<bool>(bools, key, ret);
}

bool ConfigStorage::GetString(const std::string &key, std::string *ret)
{
	std::shared_lock lock(ms);
	return GetOrDefault<std::string>(strings, key, ret);
}

bool ConfigStorage::GetInteger(const std::string &key, int64_t *ret)
{
	std::shared_lock lock(mi);
	return GetOrDefault<int64_t>(ints, key, ret);
}

bool ConfigStorage::GetFloat(const std::string &key, double *ret)
{
	std::shared_lock lock(mf);
	return GetOrDefault<double>(floats, key, ret);
}

bool ConfigStorage::GetVec2(const std::string &key, glm::vec2 *ret)
{
	std::shared_lock lock(mv2);
	return GetOrDefault<glm::vec2>(vec2, key, ret);
}

bool ConfigStorage::GetVec3(const std::string &key, glm::vec3 *ret)
{
	std::shared_lock lock(mv3);
	return GetOrDefault<glm::vec3>(vec3, key, ret);
}

bool ConfigStorage::GetVec4(const std::string &key, glm::vec4 *ret)
{
	std::shared_lock lock(mv4);
	return GetOrDefault<glm::vec4>(vec4, key, ret);
}

void ConfigStorage::SetBool(const std::string &key, bool value)
{
	std::lock_guard<std::shared_mutex> lock(mb);
	bools[key] = value;
}

void ConfigStorage::SetString(const std::string &key, const std::string &value)
{
	std::lock_guard<std::shared_mutex> lock(ms);
	strings[key] = value;
}

void ConfigStorage::SetInteger(const std::string &key, int64_t value)
{
	std::lock_guard<std::shared_mutex> lock(mi);
	ints[key] = value;
}

void ConfigStorage::SetFloat(const std::string &key, double value)
{
	std::lock_guard<std::shared_mutex> lock(mf);
	floats[key] = value;
}

void ConfigStorage::SetVec2(const std::string &key, glm::vec2 value)
{
	std::lock_guard<std::shared_mutex> lock(mv2);
	vec2[key] = value;
}

void ConfigStorage::SetVec3(const std::string &key, glm::vec3 value)
{
	std::lock_guard<std::shared_mutex> lock(mv3);
	vec3[key] = value;
}

void ConfigStorage::SetVec4(const std::string &key, glm::vec4 value)
{
	std::lock_guard<std::shared_mutex> lock(mv4);
	vec4[key] = value;
}

bool ConfigStorage::GetOrSetBool(const std::string &key, bool defaultValue)
{
	bool ret;
	if (GetBool(key, &ret)) {
		return ret;
	} else {
		SetBool(key, defaultValue);
		return defaultValue;
	}
}

std::string ConfigStorage::GetOrSetString(const std::string &key,
										  std::string defaultValue)
{
	std::string ret;
	if (GetString(key, &ret)) {
		return ret;
	} else {
		SetString(key, defaultValue);
		return defaultValue;
	}
}

int64_t ConfigStorage::GetOrSetInteger(const std::string &key,
									   int64_t defaultValue)
{
	int64_t ret;
	if (GetInteger(key, &ret)) {
		return ret;
	} else {
		SetInteger(key, defaultValue);
		return defaultValue;
	}
}

double ConfigStorage::GetOrSetFloat(const std::string &key, double defaultValue)
{
	double ret;
	if (GetFloat(key, &ret)) {
		return ret;
	} else {
		SetFloat(key, defaultValue);
		return defaultValue;
	}
}

glm::vec2 ConfigStorage::GetOrSetVec2(const std::string &key,
									  glm::vec2 defaultValue)
{
	glm::vec2 ret;
	if (GetVec2(key, &ret)) {
		return ret;
	} else {
		SetVec2(key, defaultValue);
		return defaultValue;
	}
}

glm::vec3 ConfigStorage::GetOrSetVec3(const std::string &key,
									  glm::vec3 defaultValue)
{
	glm::vec3 ret;
	if (GetVec3(key, &ret)) {
		return ret;
	} else {
		SetVec3(key, defaultValue);
		return defaultValue;
	}
}

glm::vec4 ConfigStorage::GetOrSetVec4(const std::string &key,
									  glm::vec4 defaultValue)
{
	glm::vec4 ret;
	if (GetVec4(key, &ret)) {
		return ret;
	} else {
		SetVec4(key, defaultValue);
		return defaultValue;
	}
}

std::string ConfigStorage::AllConfigsCommand()
{
	std::ostringstream ss;

	mb.lock();
	for (const auto &it : bools) {
		if (ss.tellp() > 0)
			ss << "\n";
		ss << "setb " << it.first << " " << (it.second ? 1 : 0);
	}
	mb.unlock();

	ms.lock();
	for (const auto &it : strings) {
		if (ss.tellp() > 0)
			ss << "\n";
		ss << "sets " << it.first << " \"" << it.second << "\"";
	}
	ms.unlock();

	mi.lock();
	for (const auto &it : ints) {
		if (ss.tellp() > 0)
			ss << "\n";
		ss << "seti " << it.first << " " << it.second;
	}
	mi.unlock();

	ss << std::fixed;

	mf.lock();
	for (const auto &it : floats) {
		if (ss.tellp() > 0)
			ss << "\n";
		ss << "setf " << it.first << " " << it.second;
	}
	mf.unlock();

	mv2.lock();
	for (const auto &it : vec2) {
		if (ss.tellp() > 0)
			ss << "\n";
		ss << "setv2 " << it.first << " " << it.second.x << " " << it.second.y;
	}
	mv2.unlock();

	mv3.lock();
	for (const auto &it : vec3) {
		if (ss.tellp() > 0)
			ss << "\n";
		ss << "setv3 " << it.first << " " << it.second.x << " " << it.second.y
		   << " " << it.second.z;
	}
	mv3.unlock();

	mv4.lock();
	for (const auto &it : vec4) {
		if (ss.tellp() > 0)
			ss << "\n";
		ss << "setv4 " << it.first << " " << it.second.w << " " << it.second.x
		   << " " << it.second.y << " " << it.second.z;
	}
	mv4.unlock();

	return ss.str();
}

void ConfigStorage::InitialiseCommands(CommandParser *com)
{
	com->RegisterCustomCommand(
		{"setb"},
		"Sets named config variable with boolean value (0 or 1)\n"
		"arguments:\n"
		"  - variable name\n"
		"  - new boolean value",
		[this](const std::vector<std::string> &args) {
			if (args.size() < 3) {
				LOG_ERROR("Not enaught arguments to setb");
				return;
			}
			SetBool(args[1], stoi(args[2]));
		});

	com->RegisterCustomCommand({"sets"},
							   "Sets named config variable with string value\n"
							   "arguments:\n"
							   "  - variable name\n"
							   "  - new string value",
							   [this](const std::vector<std::string> &args) {
								   if (args.size() < 3) {
									   LOG_ERROR(
										   "Not enaught arguments to sets");
									   return;
								   }
								   SetString(args[1], args[2]);
							   });

	com->RegisterCustomCommand({"seti"},
							   "Sets named config variable with integer value\n"
							   "arguments:\n"
							   "  - variable name\n"
							   "  - new integer value",
							   [this](const std::vector<std::string> &args) {
								   if (args.size() < 3) {
									   LOG_ERROR(
										   "Not enaught arguments to seti");
									   return;
								   }
								   SetInteger(args[1], std::stoll(args[2]));
							   });

	com->RegisterCustomCommand({"setf"},
							   "Sets named config variable with float value\n"
							   "arguments:\n"
							   "  - variable name\n"
							   "  - new float value",
							   [this](const std::vector<std::string> &args) {
								   if (args.size() < 3) {
									   LOG_ERROR(
										   "Not enaught arguments to setf");
									   return;
								   }
								   SetFloat(args[1], std::stod(args[2]));
							   });

	com->RegisterCustomCommand(
		{"setv2"},
		"Sets named config variable with vec2 value\n"
		"arguments:\n"
		"  - variable name\n"
		"  - new X value\n"
		"  - new Y value",
		[this](const std::vector<std::string> &args) {
			if (args.size() < 4) {
				LOG_ERROR("Not enaught arguments to setv2");
				return;
			}
			SetVec2(args[1], {std::stof(args[2]), std::stof(args[3])});
		});

	com->RegisterCustomCommand(
		{"setv3"},
		"Sets named config variable with vec3 value\n"
		"arguments:\n"
		"  - variable name\n"
		"  - new X value\n"
		"  - new Y value\n"
		"  - new Z value",
		[this](const std::vector<std::string> &args) {
			if (args.size() < 4) {
				LOG_ERROR("Not enaught arguments to setv3");
				return;
			}
			SetVec3(args[1], {std::stof(args[2]), std::stof(args[3]),
							  std::stof(args[4])});
		});

	com->RegisterCustomCommand(
		{"setv4"},
		"Sets named config variable with vec4 value\n"
		"arguments:\n"
		"  - variable name\n"
		"  - new W value\n"
		"  - new X value\n"
		"  - new Y value\n"
		"  - new Z value",
		[this](const std::vector<std::string> &args) {
			if (args.size() < 4) {
				LOG_ERROR("Not enaught arguments to setv4");
				return;
			}
			SetVec4(args[1], {std::stof(args[2]), std::stof(args[3]),
							  std::stof(args[4]), std::stof(args[5])});
		});

	com->RegisterCustomCommand(
		{"print_all_variables", "print_all"}, "Prints all variables",
		[this](const std::vector<std::string> &args) {
			std::string str = AllConfigsCommand();
			std::string ret = std::replace_all(str, "\n", "\n  ");
			printf("All stored variables:\n  %s\n", ret.c_str());
		});

	com->RegisterCustomCommand(
		{"save_variables_to_file"},
		"Saves all variables to a file\n"
		"arguments:\n"
		"  - file name to save variables",
		[this](const std::vector<std::string> &args) {
			if (args.size() < 2) {
				LOG_ERROR("Not enaught arguments to save_variables_to_file");
				return;
			}
			FILE *file = fopen(args[1].c_str(), "wb");
			if (file) {
				std::string str = AllConfigsCommand();
				fprintf(file, "%s\n", str.c_str());
				fclose(file);
			} else {
				LOG_ERROR("Failed to open file `%s` for writing",
						  args[1].c_str());
			}
		});
}
