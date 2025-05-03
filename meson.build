project(
    'aurgh',
    'cpp',
    default_options: [
        'cpp_std=c++23',
        'optimization=2',
        'warning_level=1'
    ]
)

src = files(
    'src/package/card.cc',
    'src/package/tab.cc',

    'src/arg_parser.cc',
    'src/aur_client.cc',
    'src/logger.cc',
    'src/main.cc',
    'src/process.cc',
    'src/utils.cc'
)

deps = [
    dependency('gtkmm-3.0'),
    dependency('glibmm-2.4'),
    dependency('libcurl'),
    dependency('jsoncpp'),
    dependency('libalpm'),
]

cpp_args = [
    '-g',
    '-Wall',
    '-Wextra',
    '-DGDK_DISABLE_DEPRECATED',
    '-DGTK_DISABLE_DEPRECATED',
]

executable(
    'aurgh',
    src,
    dependencies: deps,
    cpp_args: cpp_args,
    include_directories: include_directories('include')
)