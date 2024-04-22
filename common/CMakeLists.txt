cmake_minimum_required(VERSION 3.19)
project(TheGameRPG-common)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
else()
endif()

set(FLECS_SHARED OFF)
add_compile_definitions(FLECS_CUSTOM_BUILD FLECS_SYSTEM FLECS_CPP)

find_package(Bullet REQUIRED)
include_directories(${BULLET_INCLUDE_DIR})

message(MESSAGE ${BULLET_INCLUDE_DIR})

include_directories(../ICon7-godot-client/ICon7/bitscpp/include)
include_directories(../ICon7-godot-client/ICon7/include)
include_directories(include)
include_directories(flecs)

# Library compilation

aux_source_directory(./src/ src_files)
aux_source_directory(./include/ include_files)
add_library(common STATIC
	${src_files}
	${include_files}
	flecs/flecs.c
)
target_link_libraries(common PUBLIC icon7)
target_link_libraries(common PRIVATE ${BULLET_LIBRARIES})
