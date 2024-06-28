cmake_minimum_required(VERSION 3.19)
project(TheGameRPG-frontend)

include(../precompile_headers.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
else()
endif()

include_directories(../godot-cpp/include)
include_directories(../common/include)
include_directories(../client/include)
include_directories(game_frontend)

file(GLOB_RECURSE src_files_cpp "game_frontend/*.cpp")
add_library(game_frontend SHARED
	${src_files_cpp}
)
target_link_libraries(game_frontend PUBLIC client)
target_link_libraries(game_frontend PUBLIC godot::cpp)

if(THEGAMERPG_PRECOMPILE_HEADERS )
	add_common_precompiled_headers(game_frontend)

	add_precompiled_header_glob_recurse(game_frontend
		"../thirdparty/godot-cpp/gen/include*.hpp")
endif()

if(UNIX)
	add_custom_command(TARGET game_frontend POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:game_frontend>
		${PROJECT_SOURCE_DIR}/libgame_frontend.so
		COMMENT "Copying libgame_frontend.so"
	)
else()
	add_custom_command(TARGET game_frontend POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:game_frontend>
		${PROJECT_SOURCE_DIR}/libgame_frontend.dll
		COMMENT "Copying libgame_frontend.dll"
	)
endif()

if (THEGAMERPG_TIME_REPORT)
	target_compile_options(game_frontend PUBLIC -ftime-report)
endif()
