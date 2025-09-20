cmake_minimum_required(VERSION 3.19)
project(TheGameRPG-server)

include(../precompile_headers.cmake)

option(THEGAMERPG_SERVER_SCRIPTS_PCH "Enable pch for server scripts" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

if(ENABLE_LTO)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

include_directories(../common/include)
include_directories(include)

file(GLOB_RECURSE src_files "src/*.cpp")
file(GLOB_RECURSE script_files "scripts/*.cpp")

message(script_files)

add_library(scripts SHARED
	${script_files}
)

if (WIN32)
	add_library(server_lib SHARED
		${src_files}
	)
	target_link_libraries(server_lib PUBLIC common)

	add_executable(server
		ServerMain.cpp
	)
	target_link_libraries(server server_lib)

	target_link_libraries(scripts server_lib)
else()
	add_library(server_lib STATIC
		${src_files}
	)
	target_link_libraries(server_lib PUBLIC common)
	add_library(server_lib_shared SHARED
		${src_files}
	)
	target_link_libraries(server_lib_shared PUBLIC common)

	add_executable(server
		ServerMain.cpp
	)
	target_link_libraries(server server_lib)
	# ENABLE_EXPORTS allows for dlopen(scripts) to link against symbols from within
	# 				 server executable file
	set_target_properties(server PROPERTIES ENABLE_EXPORTS ON)

	if (THEGAMERPG_SERVER_SCRIPTS_PCH)
		add_precompiled_header_glob_recurse(scripts "scripts/BigHeader.hpp")
	endif()

	if (THEGAMERPG_TIME_REPORT)
		target_compile_options(server PUBLIC -ftime-report)
		target_compile_options(scripts PUBLIC -ftime-report)
	endif()

	if (THEGAMERPG_PRECOMPILE_HEADERS)
		add_precompiled_header_icon7(scripts)
		add_precompiled_header_glob_recurse(scripts "include/*.hpp")
		add_precompiled_header_glob_recurse(scripts "../common/include/*.hpp")

		add_precompiled_header_icon7(server)
		add_precompiled_header_glob_recurse(server "include/*.hpp")
		add_precompiled_header_glob_recurse(server "../common/include/*.hpp")

		add_precompiled_header_icon7(server_lib_shared)
		add_precompiled_header_glob_recurse(server_lib_shared "include/*.hpp")
		add_precompiled_header_glob_recurse(server_lib_shared "../common/include/*.hpp")

		add_precompiled_header_icon7(server_lib)
		add_precompiled_header_glob_recurse(server_lib "include/*.hpp")
		add_precompiled_header_glob_recurse(server_lib "../common/include/*.hpp")
	endif()

	add_common_precompiled_headers(server_lib)
	add_precompiled_header_thirdparty(server_lib)

	add_common_precompiled_headers(server_lib_shared)
	add_precompiled_header_thirdparty(server_lib_shared)
endif()

add_common_precompiled_headers(server)
add_precompiled_header_thirdparty(server)

add_common_precompiled_headers(scripts)
add_precompiled_header_thirdparty(scripts)

add_custom_target(game_server DEPENDS server scripts)

if (THEGAMERPG_MAP_EDITOR)
	if(UNIX)
	else()
		add_custom_command(TARGET server_lib POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:server_lib>
			${PROJECT_SOURCE_DIR}/../frontend/libserver_lib.dll
			COMMENT "Copying libserver_lib.dll"
		)
	endif()
endif()
