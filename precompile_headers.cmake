function(add_precompiled_header_icon7 target)
	add_precompiled_header_glob_recurse(${target} "../ICon7/include/*.hpp")
	add_precompiled_header_glob_recurse(${target} "../ICon7/include/bitscpp/*.hpp")
endfunction()

function(add_precompiled_header_thirdparty target)
	add_precompiled_header_glob_recurse(${target} "../thirdparty/flecs/distr/*.h")
	target_precompile_headers(${target} PUBLIC
		"../ICon7/concurrentqueue/concurrentqueue.h"
	)
endfunction()

function(add_precompiled_header_godot target)
	add_precompiled_header_glob_recurse(${target} "../thirdparty/godot-cpp/include/godot_cpp/*")
	add_precompiled_header_glob_recurse(${target} "./thirdparty/godot-cpp/gen/include/godot_cpp/*")
endfunction()

function(add_precompiled_header_glob_recurse target pattern)
	file(GLOB_RECURSE files ${pattern})
	target_precompile_headers(${target} PUBLIC ${files})
endfunction()
