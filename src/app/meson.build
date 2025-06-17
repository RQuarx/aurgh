aurgh_src = files(
    'package/card.cc',
    'package/client.cc',
    'package/tab.cc',

    'config.cc',
    'dialog.cc',
    'logger.cc',
    'main.cc',
)


executable(
    'aurgh',
    aurgh_src,
    include_directories: include_directories('../../include/app'),
    dependencies: [ dependency('shared') ]
)