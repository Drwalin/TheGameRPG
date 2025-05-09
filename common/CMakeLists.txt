cmake_minimum_required(VERSION 3.19)
project(TheGameRPG-common)

include(../precompile_headers.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(ENABLE_LTO)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

include_directories(../ICon7-godot-client/ICon7/bitscpp/include)
include_directories(../ICon7-godot-client/ICon7/include)
include_directories(include)

add_compile_definitions(FLECS_CUSTOM_BUILD FLECS_SYSTEM FLECS_CPP
	FLECS_LOG FLECS_APP FLECS_META FLECS_META_C FLECS_EXPR FLECS_DOC)
add_library(flecs STATIC ../thirdparty/flecs/flecs.c)
set_target_properties(flecs PROPERTIES ENABLE_EXPORTS ON)

file(GLOB_RECURSE src_files "src/*.cpp")
add_library(common STATIC
	${src_files}
)
target_link_libraries(common PUBLIC icon7)
target_link_libraries(common PUBLIC flecs)
target_link_libraries(common PUBLIC collision3d)
target_link_libraries(common PUBLIC BulletCollision)
target_link_libraries(common PUBLIC Bullet3Collision)
target_link_libraries(common PUBLIC Bullet3Geometry)
target_link_libraries(common PUBLIC Bullet3Common)
target_link_libraries(common PUBLIC LinearMath)
set_target_properties(common PROPERTIES ENABLE_EXPORTS ON)

if(THEGAMERPG_PRECOMPILE_HEADERS )
	add_common_precompiled_headers(common)
endif()

if (THEGAMERPG_TIME_REPORT)
	target_compile_options(common PUBLIC -ftime-report)
	target_compile_options(flecs PUBLIC -ftime-report)
endif()
