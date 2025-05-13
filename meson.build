project(
    'aurgh',
    'cpp',
    version: '0.1.1',
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


if get_option('use-gtk4')
    add_project_dependencies(
        dependency('gtkmm-4.0'), language: 'cpp'
    )
    add_project_dependencies(
        dependency('glibmm-2.68'), language: 'cpp'
    )
else
    add_project_dependencies(
        dependency('gtkmm-3.0'), language: 'cpp'
    )
    add_project_dependencies(
        dependency('glibmm-2.4'), language: 'cpp'
    )
endif

deps = [
    dependency('libcurl'),
    dependency('jsoncpp'),
    dependency('libalpm'),
]

foreach dep : deps
    add_project_dependencies(dep, language: 'cpp')
endforeach



add_project_arguments([
    '-DGDK_DISABLE_DEPRECATED',
    '-DGTK_DISABLE_DEPRECATED',
    '-DAPP_VERSION="@0@"'.format(meson.project_version()),
    '-DAPP_NAME="@0@"'.format(meson.project_name()),
    '-DPRETTY_LOGGING=@0@'.format(get_option('pretty-logging')),
], language: 'cpp')


executable(
    'aurgh',
    src,
    include_directories: include_directories('include', 'include/package')
)