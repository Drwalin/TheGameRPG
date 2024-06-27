#pragma once

#include <string_view>
#include <vector>
#include <string>

inline void TrimInplace(std::string_view &str,
						const std::string &whitespace = " \t\n")
{
	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == std::string::npos) {
		str = "";
	} else {
		const auto strEnd = str.find_last_not_of(whitespace);
		str = std::string_view(str.data() + strBegin, strEnd - strBegin + 1);
	}
}

inline void TrimInplace(std::string &str,
						const std::string &whitespace = " \t\n")
{
	std::string_view sv(str);
	TrimInplace(sv, whitespace);
	str.resize(sv.data() - str.data() + sv.size());
	str.erase(0, sv.data() - str.data());
}

namespace std
{
inline std::string to_string(std::string_view sv)
{
	return std::string(sv.data(), sv.size());
}
} // namespace std

inline std::vector<std::string_view>
ConvertToArgsList(std::string_view str)
{
	std::vector<std::string_view> args;
	while (str.size() > 0) {
		size_t offset = str.find_first_not_of(" \t\n");
		if (offset == std::string::npos) {
			break;
		}
		str = str.substr(offset);
		if (str[0] == '"') {
			size_t end = str.find_first_of("\"", 1);
			args.push_back(str.substr(1, end-1));
			if (end == std::string::npos) {
				break;
			}
			str = str.substr(end+1);
		} else if (str[0] == '\'') {
			size_t end = str.find_first_of("'", 1);
			args.push_back(str.substr(1, end-1));
			if (end == std::string::npos) {
				break;
			}
			str = str.substr(end+1);
		} else {
			size_t end = str.find_first_of(" \t\n");
			args.push_back(str.substr(0, end));
			if (end == std::string::npos) {
				break;
			}
			str = str.substr(end);
		}
	}
	return args;
}
