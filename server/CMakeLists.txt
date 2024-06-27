cmake_minimum_required(VERSION 3.19)
project(TheGameRPG-server)

include(../precompile_headers.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

include_directories(../ICon7-godot-client/ICon7/bitscpp/include)
include_directories(../ICon7-godot-client/ICon7/include)
include_directories(../common/include)
include_directories(include)

file(GLOB_RECURSE src_files "src/*.cpp")

add_executable(server
	${src_files}
)
set_target_properties(server PROPERTIES ENABLE_EXPORTS ON)
target_link_libraries(server common)
if(THEGAMERPG_PRECOMPILE_HEADERS )
	add_common_precompiled_headers(server)
endif()

file(GLOB_RECURSE script_files "scripts/*.cpp")
add_library(scripts SHARED
	${script_files}
)

add_custom_target(game_server DEPENDS server scripts)
