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

core_deps = [
    dependency('libcurl'),
    dependency('jsoncpp'),
    dependency('libalpm'),
]

gtk3_deps = [
    'gtkmm-3.0',
    'glibmm-2.4',
]

gtk4_deps = [
    'gtkmm-4.0',
    'glibmm-2.68',
]

foreach dep : core_deps
    add_project_dependencies(dep, language: 'cpp')
endforeach

use_gtk4 = get_option('use-gtk4')
if use_gtk4
    foreach dep : gtk4_deps
        add_project_dependencies(dependency(dep), language: 'cpp')
    endforeach
else
    foreach dep : gtk3_deps
        add_project_dependencies(dependency(dep), language: 'cpp')
    endforeach
endif


add_project_arguments([
    '-DGDK_DISABLE_DEPRECATED',
    '-DGTK_DISABLE_DEPRECATED',
    '-DAPP_VERSION="@0@"'.format(meson.project_version()),
    '-DAPP_NAME="@0@"'.format(meson.project_name()),
    '-DPRETTY_LOGGING=@0@'.format(get_option('pretty-logging')),
    '-DGTK4=' + (use_gtk4 ? 'true' : 'false'),
    '-DGTK3=' + (use_gtk4 ? 'false' : 'true'),
], language: 'cpp')

subdir('helper')

executable(
    'aurgh',
    src,
    include_directories: include_directories('include', 'include/package')
)