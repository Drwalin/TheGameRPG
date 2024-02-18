cmake_minimum_required(VERSION 3.19)
project(the_game_rpg)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(ICon7-godot-client)
add_subdirectory(server)
add_subdirectory(client)

