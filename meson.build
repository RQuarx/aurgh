project(
    'aurgh',
    'cpp',
    default_options: ['cpp_std=c++23', 'warning_level=3'],
    version: '1.14.3-alpha',
    license: 'GPL-3.0-or-later',
    license_files: ['COPYING'],
    meson_version: '>=1.9.0'
)

cxx_args = [
    '-DAPP_NAME="' + meson.project_name() + '"',
    '-DAPP_VERSION="' + meson.project_version() + '"',
    '-DAPP_ID="org.kei.aurgh"'
]

add_project_dependencies(
    [
        dependency('fontconfig'),
        dependency('gtkmm-3.0'),
        dependency('libcurl'),
        dependency('jsoncpp'),
        dependency('libalpm'),
    ],
    language: 'cpp',
)

include = include_directories('include', '.')

subdir('data')
subdir('src')

gnome = import('gnome')

resource = gnome.compile_resources(
    'aurgh-resource',
    'resources.xml',
    source_dir: meson.current_source_dir(),
    c_name: 'aurgh',
)

executable(
    'aurgh',
    [ source_files, resource ],
    include_directories: include,
    cpp_args: cxx_args,
    install: true,
)