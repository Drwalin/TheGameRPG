def Settings( **kwargs ):
  return {
    'flags': ['-x', 'c++', '-Wall', '-pedantic', '-Isrc',
    '-Iinclude',
    '-I../ICon7-godot-client/ICon7/bitscpp/include',
    '-I../ICon7-godot-client/ICon7/uSockets/src/libusockets.h',
    '-I../ICon7-godot-client/ICon7/include', '-std=c++17', '-I/usr/include',
    '-I../common/flecs/include',
    '-I../common/include'],
  }
