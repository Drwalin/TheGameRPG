def Settings( **kwargs ):
  return {
    'flags': ['-x', 'c++', '-Wall', '-pedantic', '-Isrc',
    '-Iinclude',
    '-I../ICon7-godot-client/ICon7/bitscpp/include',
    '-I../ICon7-godot-client/ICon7/uSockets/src/libusockets.h',
    '-I../ICon7-godot-client/ICon7/include', '-std=c++20', '-I/usr/include',
    '-I./flecs/include',
    '-I/usr/include/bullet'],
  }
