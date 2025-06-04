aurgh_src = files(
    'src/package/card.cc',
    'src/package/client.cc',
    'src/package/tab.cc',

    'src/config.cc',
    'src/logger.cc',
    'src/main.cc',
)


executable(
    'aurgh',
    aurgh_src,
    include_directories: include_directories(
        'include',
    ),
    dependencies: [ dependency('shared') ]
)