cmake_minimum_required(VERSION 3.19)
project(TheGameRPG-frontend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

include_directories(../godot-cpp/include)
include_directories(../common/include)
include_directories(../client/include)
include_directories(game_frontend)

aux_source_directory(./game_frontend src_files)
add_library(game_frontend SHARED
	${src_files}
)
target_link_libraries(game_frontend PUBLIC client)
target_link_libraries(game_frontend PUBLIC godot::cpp)

add_custom_command(TARGET game_frontend POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:game_frontend>
	${PROJECT_SOURCE_DIR}/bin/libgame_frontend.so
	COMMENT "dummy"
)
