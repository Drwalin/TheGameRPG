macro(add_precompiled_header_glob_recurse target pattern)
	file(GLOB_RECURSE files ${pattern})
	target_precompile_headers(${target} PUBLIC ${files})
endmacro()

macro(add_common_precompiled_headers target)
	target_precompile_headers(${target} PUBLIC <map> <unordered_map> <vector>
		<string> <functional> <string_view> <fstream> <sstream> <thread>
		<atomic> <mutex> <queue> <list> <array> <valarray> <set>
		<unordered_set> <algorithm> <ios> <ostream> <streambuf> <utility>
		<stack> <new> <memory> <limits> <iterator> <complex> <bitset> <locale>
		<ostream> <istream> <future> <tuple> <regex> <type_traits>
		<forward_list> <initializer_list> <condition_variable> <random> <chrono>
		<shared_mutex> <filesystem>
		<memory_resource> <cstring>)
	
	add_precompiled_header_glob_recurse(${target} "../ICon7/include/*.hpp")
	add_precompiled_header_glob_recurse(${target} "../ICon7/bitscpp/*.hpp")
endmacro()
