cmake_minimum_required(VERSION 3.19)
project(TheGameRPG-client)

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

aux_source_directory(./include include_files)
aux_source_directory(./src src_files)
add_library(client SHARED
	${include_files}
	${src_files}
)
target_link_libraries(client PUBLIC common)

add_custom_command(TARGET client POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:client>
	${PROJECT_SOURCE_DIR}/../frontend/bin/libclient.so
	COMMENT "Copying libclient.so"
)
