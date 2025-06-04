aurgh_src = files(
    'src/package/card.cc',
    'src/package/tab.cc',

    'src/aur_client.cc',
    'src/config.cc',
    'src/logger.cc',
    'src/main.cc',
)


executable(
    'aurgh',
    aurgh_src,
    include_directories: include_directories(
        'include/package',
        'include',
    ),
    dependencies: [ dependency('shared') ]
)