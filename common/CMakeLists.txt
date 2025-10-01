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

add_compile_definitions(FLECS_CUSTOM_BUILD FLECS_SYSTEM FLECS_CPP
	FLECS_LOG FLECS_APP FLECS_META FLECS_META_C FLECS_EXPR FLECS_DOC)
add_library(flecs STATIC ../thirdparty/flecs/distr/flecs.c)
set_target_properties(flecs PROPERTIES ENABLE_EXPORTS ON)

include_directories(../ICon7-godot-client/ICon7/bitscpp/include)
include_directories(../ICon7-godot-client/ICon7/include)
include_directories(include)

file(GLOB_RECURSE src_files "src/*.cpp")
file(GLOB_RECURSE srheader_files "inlude/*.hpp")
add_library(common STATIC
	${src_files}
	${srheader_files}
)
target_link_libraries(common PUBLIC icon7)
target_link_libraries(common PUBLIC flecs)
target_link_libraries(common PUBLIC collision3d)
set_target_properties(common PROPERTIES ENABLE_EXPORTS ON)

if(THEGAMERPG_PRECOMPILE_HEADERS)
	add_precompiled_header_thirdparty(common)
endif()

add_common_precompiled_headers(common)
add_precompiled_header_thirdparty(common)

if (THEGAMERPG_TIME_REPORT)
	target_compile_options(common PUBLIC -ftime-report)
	target_compile_options(flecs PUBLIC -ftime-report)
endif()
