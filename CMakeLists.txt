cmake_minimum_required(VERSION 3.19)
project(the_game_rpg)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_COLOR_DIAGNOSTICS ON)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
else()
endif()

option(THEGAMERPG_PRECOMPILE_HEADERS "Enable pracompiled headers" OFF)

include_directories(thirdparty/flecs/include)

find_package(Bullet REQUIRED)
include_directories(${BULLET_INCLUDE_DIR})
include_directories(${BULLET_INCLUDE_DIR}/LinearMath)

add_subdirectory(thirdparty/godot-cpp)

set(ICON7_IGNORE_COMMON_LOG_PATH 1)
add_subdirectory(ICon7)
target_compile_definitions(icon7 PUBLIC ICON7_LOG_IGNORE_COMMON_PATH="${PROJECT_SOURCE_DIR}")

include_directories(ICon7/bitscpp/include)
include_directories(ICon7/include)

add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(frontend)
