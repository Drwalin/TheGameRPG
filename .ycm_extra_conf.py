def Settings( **kwargs ):
  return {
    'flags': ['-x', 'c++', '-Wall', '-pedantic', '-Isrc',
    '-std=c++20',
    '-I/usr/include',
    '-I/usr/include/bullet'
    '-I./ICon7-godot-client/ICon7/bitscpp/include',
    '-I./ICon7-godot-client/ICon7/uSockets/src/libusockets.h',
    '-I./ICon7-godot-client/ICon7/include',
    '-I./common/include',
    '-I./server/include',
    '-I./client/src/include',
    '-I./common/flecs/include'],
  }
