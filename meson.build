project(
    'aurgh',
    'cpp',
    version: run_command(
        'grep', '-Po', 'APP_VERSION\\s*=\\s*"\K[0-9.]+', 'src/main.cc', check: true
    ).stdout().strip(),
    default_options: [
        'cpp_std=c++23',
        'optimization=2',
        'warning_level=3'
    ],
    license: 'GPL-3.0-or-later',
    license_files: 'LICENSE'
)

src = files(
    'src/package/card.cc',
    'src/package/tab.cc',

    'src/arg_parser.cc',
    'src/aur_client.cc',
    'src/config.cc',
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