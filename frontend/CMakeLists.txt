cmake_minimum_required(VERSION 3.19)
project(TheGameRPG-frontend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

include_directories(../ICon7-godot-client/ICon7/bitscpp/include)
include_directories(../ICon7-godot-client/ICon7/include)
include_directories(../ICon7-godot-client/godot-cpp/include)
include_directories(../common/include)
include_directories(src/include)

# Library compilation
add_custom_command(OUTPUT
		${PROJECT_SOURCE_DIR}/bin/libicon7_godot_client.so
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:icon7_godot_client>
		${PROJECT_SOURCE_DIR}/bin/
    DEPENDS icon7_godot_client
)
# Create target which consume the command via DEPENDS.
add_custom_target(copy_files ALL
    DEPENDS ${PROJECT_SOURCE_DIR}/bin/libicon7_godot_client.so
)



aux_source_directory(./game_client src_files)
add_library(game_client SHARED
	${src_files}
)
target_link_libraries(game_client PUBLIC icon7_godot_client)
target_link_libraries(game_client PUBLIC client)

add_custom_command(TARGET game_client POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:game_client>
	${PROJECT_SOURCE_DIR}/bin/libgame_client.so
	COMMENT "dummy"
)
