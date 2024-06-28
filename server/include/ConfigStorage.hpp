#pragma once

#include <unordered_map>
#include <string>
#include <shared_mutex>

#include <glm/glm.hpp>

class ConfigStorage
{
public:
	ConfigStorage(class ServerCore *serverCore);
	~ConfigStorage();

	void InitialiseCommands(class CommandParser *commandsParser);

	bool GetBool(const std::string &key, bool *ret);
	bool GetString(const std::string &key, std::string *ret);
	bool GetInteger(const std::string &key, int64_t *ret);
	bool GetFloat(const std::string &key, double *ret);
	bool GetVec2(const std::string &key, glm::vec2 *ret);
	bool GetVec3(const std::string &key, glm::vec3 *ret);
	bool GetVec4(const std::string &key, glm::vec4 *ret);

	bool GetOrSetBool(const std::string &key, bool defaultValue);
	std::string GetOrSetString(const std::string &key,
							   std::string defaultValue);
	int64_t GetOrSetInteger(const std::string &key, int64_t defaultValue);
	double GetOrSetFloat(const std::string &key, double defaultValue);
	glm::vec2 GetOrSetVec2(const std::string &key, glm::vec2 defaultValue);
	glm::vec3 GetOrSetVec3(const std::string &key, glm::vec3 defaultValue);
	glm::vec4 GetOrSetVec4(const std::string &key, glm::vec4 defaultValue);

	void SetBool(const std::string &key, bool value);
	void SetString(const std::string &key, const std::string &value);
	void SetInteger(const std::string &key, int64_t value);
	void SetFloat(const std::string &key, double value);
	void SetVec2(const std::string &key, glm::vec2 value);
	void SetVec3(const std::string &key, glm::vec3 value);
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
