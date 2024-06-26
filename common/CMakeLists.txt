cmake_minimum_required(VERSION 3.19)
project(TheGameRPG-common)

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

include_directories(../ICon7-godot-client/ICon7/bitscpp/include)
include_directories(../ICon7-godot-client/ICon7/include)
include_directories(include)

add_compile_definitions(FLECS_CUSTOM_BUILD FLECS_SYSTEM FLECS_CPP)
add_library(flecs STATIC ../thirdparty/flecs/flecs.c)

file(GLOB_RECURSE src_files "src/*.cpp")
add_library(common STATIC
	${src_files}
)
target_link_libraries(common PUBLIC icon7)
target_link_libraries(common PUBLIC flecs)
target_link_libraries(common PRIVATE ${BULLET_LIBRARIES})

if(THEGAMERPG_PRECOMPILE_HEADERS )
	add_common_precompiled_headers(common)
endif()
