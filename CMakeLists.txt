cmake_minimum_required(VERSION 3.19)
project(the_game_rpg)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

include_directories(common/flecs)

set(ICON7_IGNORE_COMMON_LOG_PATH 1)
add_subdirectory(ICon7-godot-client)

target_compile_definitions(icon7 PUBLIC ICON7_LOG_IGNORE_COMMON_PATH="${PROJECT_SOURCE_DIR}")

add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(frontend)

