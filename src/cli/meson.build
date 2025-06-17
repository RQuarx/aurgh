helper_src = files(
    'alpm.cc',
    'log.cc',
    'main.cc',
)


executable(
    'helper',
    helper_src,
    include_directories: include_directories('../../include/cli'),
    dependencies: [ dependency('shared') ]
)