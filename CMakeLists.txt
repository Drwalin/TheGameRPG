cmake_minimum_required(VERSION 3.19)
project(the_game_rpg)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_COLOR_DIAGNOSTICS ON)

option(THEGAMERPG_PRECOMPILE_HEADERS "Enable pracompiled headers" OFF)
option(THEGAMERPG_TIME_REPORT "Enable time report for compilation" OFF)
option(THEGAMERPG_MAP_EDITOR "Enable map editor" OFF)
option(THEGAMERPG_UNITY_BUILD "Enable unity build" OFF)

if(ENABLE_LTO)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

include_directories(thirdparty/flecs/include)

add_subdirectory(thirdparty/Collision3D)
include_directories(thirdparty/Collision3D)
include_directories(thirdparty/Collision3D/SpatialPartitioning/glm)

set(ICON7_IGNORE_COMMON_LOG_PATH 1)
add_subdirectory(ICon7)
target_compile_definitions(icon7 PUBLIC ICON7_LOG_IGNORE_COMMON_PATH="${PROJECT_SOURCE_DIR}")

set(CMAKE_UNITY_BUILD ON)
add_subdirectory(thirdparty/godot-cpp)
unset(CMAKE_UNITY_BUILD)

if (THEGAMERPG_UNITY_BUILD)
	set(CMAKE_UNITY_BUILD ON)
else()
	unset(CMAKE_UNITY_BUILD)
endif()

include_directories(ICon7/bitscpp/include)
include_directories(ICon7/include)

add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(frontend)

add_custom_target(game ALL DEPENDS server scripts game_frontend)
add_custom_target(editor ALL DEPENDS map_editor game_frontend)

add_custom_target(thirdparty_libs ALL DEPENDS
	rpmalloc 
	uSockets
	flecs
	godot::cpp
)
