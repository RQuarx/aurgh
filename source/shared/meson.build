shared_src = files(
    'src/utils.cc',
    'src/arg_parser.cc',
)

shared_lib = static_library(
    'shared',
    shared_src,
    include_directories: include_directories('include')
)

shared = declare_dependency(
    include_directories: include_directories('include'),
    link_with: shared_lib
)

meson.override_dependency('shared', shared)