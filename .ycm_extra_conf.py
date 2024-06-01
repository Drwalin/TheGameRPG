def Settings( **kwargs ):
  return {
    'flags': ['-x', 'c++', '-Wall', '-pedantic',
    '-std=c++20',
    '-I/usr/include',
    '-I/usr/include/bullet',
    '-Ithirdparty/godot-cpp/include',
    '-Ithirdparty/godot-cpp/gdextension',
    '-Ithirdparty/godot-cpp/gen/include',
    '-IICon7/bitscpp/include',
    '-IICon7/uSockets/src/libusockets.h',
    '-IICon7/include',
    '-Icommon/include',
    '-Iserver/include',
    '-Iclient/src/include',
    '-Ithirdparty/flecs',
    '-Ithirdparty/flecs/include'
    ],
  }
