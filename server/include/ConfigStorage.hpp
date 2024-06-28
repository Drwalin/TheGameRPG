#pragma once

#include <unordered_map>
#include <string>
#include <shared_mutex>

#include <glm/glm.hpp>

class ConfigStorage
{
public:
	ConfigStorage(class ServerCore *serverCore);

	void InitialiseCommands(class CommandParser *commandsParser);

	bool GetBool(const std::string &key, bool *ret);
	void SetBool(const std::string &key, bool value);

	bool GetString(const std::string &key, std::string *ret);
	void SetString(const std::string &key, const std::string &value);

	bool GetInteger(const std::string &key, int64_t *ret);
	void SetInteger(const std::string &key, int64_t value);

	bool GetFloat(const std::string &key, double *ret);
	void SetFloat(const std::string &key, double value);

	bool GetVec2(const std::string &key, glm::vec2 *ret);
	void SetVec2(const std::string &key, glm::vec2 value);

	bool GetVec3(const std::string &key, glm::vec3 *ret);
	void SetVec3(const std::string &key, glm::vec3 value);

	bool GetVec4(const std::string &key, glm::vec4 *ret);
	void SetVec4(const std::string &key, glm::vec4 value);

	std::string AllConfigsCommand();

public:
	std::unordered_map<std::string, bool> bools;
	std::unordered_map<std::string, std::string> strings;
	std::unordered_map<std::string, int64_t> ints;
	std::unordered_map<std::string, double> floats;
	std::unordered_map<std::string, glm::vec2> vec2;
	std::unordered_map<std::string, glm::vec3> vec3;
	std::unordered_map<std::string, glm::vec4> vec4;

	std::shared_mutex mb, ms, mi, mf, mv2, mv3, mv4;

	class ServerCore *serverCore;
};
