def Settings( **kwargs ):
  return {
    'flags': ['-x', 'c++', '-Wall', '-pedantic',
    '-std=c++20',
    '-I/usr/include',
    '-I/usr/include/bullet',
    '-Igodot-cpp/include',
    '-Igodot-cpp/gdextension',
    '-Igodot-cpp/gen/include',
    '-IICon7/bitscpp/include',
    '-IICon7/uSockets/src/libusockets.h',
    '-IICon7/include',
    '-Icommon/include',
    '-Iserver/include',
    '-Iclient/src/include',
    '-Icommon/flecs'
    ],
  }
