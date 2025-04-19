project('aurgh', 'cpp',
    default_options: ['cpp_std=c++23', 'optimization=2', 'warning_level=1']
)

src = files(
    'src/tabs/packages.cc',
    'src/arg_parser.cc',
    'src/aur_client.cc',
    'src/logger.cc',
    'src/main.cc',
    'src/utils.cc',
    'src/window.cc'
)

deps = [
    dependency('gtkmm-3.0'),
    dependency('glibmm-2.68'),
    dependency('libcurl'),
    dependency('jsoncpp')
]


executable('aurgh', src, dependencies: deps, cpp_args: [ '-g' ], include_directories: include_directories('include'))