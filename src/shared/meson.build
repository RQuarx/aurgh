shared_src = files(
    'utils.cc',
    'arg_parser.cc',
)

shared_inc = include_directories('../../include/shared')

shared_lib = static_library(
    'shared',
    shared_src,
    include_directories: shared_inc
)

shared = declare_dependency(
    include_directories: shared_inc,
    link_with: shared_lib,
)

meson.override_dependency('shared', shared)