helper_src = files(
    'src/alpm.cc',
    'src/log.cc',
    'src/main.cc',
)


executable(
    'helper',
    helper_src,
    include_directories: include_directories(
        'include',
        '../shared/include'
    ),
    dependencies: [ dependency('shared') ]
)