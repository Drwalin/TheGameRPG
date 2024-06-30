cmake_minimum_required(VERSION 3.19)
project(TheGameRPG-server)

include(../precompile_headers.cmake)

option(THEGAMERPG_SERVER_SCRIPTS_PCH "Enable pch for server scripts" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
else()
endif()

include_directories(../ICon7-godot-client/ICon7/bitscpp/include)
include_directories(../ICon7-godot-client/ICon7/include)
include_directories(../common/include)
include_directories(include)

file(GLOB_RECURSE src_files "src/*.cpp")
file(GLOB_RECURSE script_files "scripts/*.cpp")

add_library(scripts SHARED
	${script_files}
)

if (WIN32)
	add_library(server_lib SHARED
		${src_files}
	)
	target_link_libraries(server_lib common)
	add_executable(server
		ServerMain.cpp
	)
	target_link_libraries(server server_lib)
	
	target_link_libraries(scripts server_lib)
else()
	add_library(server_lib STATIC
		ServerMain.cpp
	)
	target_link_libraries(server_lib common)
	
	add_executable(server
		${src_files}
	)
	target_link_libraries(server server_lib)
	# ENABLE_EXPORTS allows for dlopen(scripts) to link against symbols from within
	# 				 server executable file
	set_target_properties(server PROPERTIES ENABLE_EXPORTS ON)
	
	if (THEGAMERPG_SERVER_SCRIPTS_PCH)
		target_precompile_headers(scripts PUBLIC scripts/BigHeader.hpp)
	endif()
	
	if (THEGAMERPG_TIME_REPORT)
		target_compile_options(server PUBLIC -ftime-report)
		target_compile_options(scripts PUBLIC -ftime-report)
	endif()
	
	if (THEGAMERPG_PRECOMPILE_HEADERS )
		add_common_precompiled_headers(scripts)
		add_precompiled_header_glob_recurse(scripts "../ICon7/include/*.hpp")
		add_precompiled_header_glob_recurse(scripts "include/*.hpp")
		add_precompiled_header_glob_recurse(scripts "../common/include/*.hpp")
	endif()
endif()

if (THEGAMERPG_PRECOMPILE_HEADERS)
	add_common_precompiled_headers(server)
endif()

add_custom_target(game_server DEPENDS server scripts)
