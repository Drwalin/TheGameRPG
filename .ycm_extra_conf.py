def Settings( **kwargs ):
  return {
    'flags': ['-x', 'c++', '-Wall', '-pedantic',
    '-std=c++20',
    '-I/usr/include',
    '-I/usr/include/bullet',
    '-I./godot-cpp/include',
    '-I./godot-cpp/gdextension',
    '-I./godot-cpp/gen/include',
    '-I./ICon7/bitscpp/include',
    '-I./ICon7/uSockets/src/libusockets.h',
    '-I./ICon7/include',
    '-I./common/include',
    '-I./server/include',
    '-I./client/src/include',
    '-I./common/flecs'
    ],
  }
