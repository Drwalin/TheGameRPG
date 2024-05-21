cmake_minimum_required(VERSION 3.19)
project(TheGameRPG-server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

include_directories(../ICon7-godot-client/ICon7/bitscpp/include)
include_directories(../ICon7-godot-client/ICon7/include)
include_directories(../common/include)
include_directories(sqlite3.45.3)
include_directories(include)

# Library compilation

aux_source_directory(./src/ src_files)
aux_source_directory(./include/ include_files)
add_executable(server
	${src_files}
	${include_files}
	sqlite3.45.3/sqlite3.c
)
target_link_libraries(server PUBLIC common)

