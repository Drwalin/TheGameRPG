cmake_minimum_required(VERSION 3.19)
project(TheGameRPG-common)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(FLECS_SHARED OFF)
add_compile_definitions(FLECS_CUSTOM_BUILD FLECS_SYSTEM FLECS_CPP)
add_subdirectory(flecs)

find_package(Bullet REQUIRED)
include_directories(${BULLET_INCLUDE_DIR})

message(MESSAGE ${BULLET_INCLUDE_DIR})

include_directories(../ICon7-godot-client/ICon7/bitscpp/include)
include_directories(../ICon7-godot-client/ICon7/include)
include_directories(include)
include_directories(flecs/include)

# Library compilation

aux_source_directory(./src/ src_files)
aux_source_directory(./include/ include_files)
add_library(common STATIC
	${src_files}
	${include_files}
)
target_link_libraries(common PUBLIC icon7)
target_link_libraries(common PUBLIC flecs)
target_link_libraries(common PRIVATE Bullet)

add_library(collision STATIC
	src/CollisionWorld.cpp)

