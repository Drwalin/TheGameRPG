#pragma once

#include <string_view>
#include <vector>
#include <string>

namespace std
{
void trim_inplace(std::string_view &str,
				 const std::string &whitespace = " \t\n");
void trim_inplace(std::string &str, const std::string &whitespace = " \t\n");
std::vector<std::string_view> convert_to_args_list(std::string_view str);
std::string replace_all(const std::string &str, const std::string &src,
					   const std::string &dst);

std::string to_string(std::string_view sv);
} // namespace std
