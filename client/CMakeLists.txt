cmake_minimum_required(VERSION 3.19)
project(TheGameRPG-client)

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
include_directories(../common/include)
include_directories(src/include)

aux_source_directory(./src src_files)
add_library(client STATIC
	${src_files}
)
target_link_libraries(client PUBLIC common)

if(THEGAMERPG_PRECOMPILE_HEADERS )
	add_precompiled_header_icon7(client)
	add_precompiled_header_glob_recurse(client "include/*.hpp")
	add_precompiled_header_glob_recurse(client "../common/include/*.hpp")
endif()

add_common_precompiled_headers(client)
add_precompiled_header_thirdparty(client)

if (THEGAMERPG_TIME_REPORT)
	target_compile_options(client PUBLIC -ftime-report)
endif()
