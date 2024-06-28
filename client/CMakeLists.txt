cmake_minimum_required(VERSION 3.19)
project(TheGameRPG-client)

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
include_directories(../common/include)
include_directories(src/include)

aux_source_directory(./src src_files)
add_library(client STATIC
	${src_files}
)
target_link_libraries(client PUBLIC common)

if(THEGAMERPG_PRECOMPILE_HEADERS )
	add_common_precompiled_headers(client)
endif()

if (THEGAMERPG_TIME_REPORT)
	target_compile_options(client PUBLIC -ftime-report)
endif()
