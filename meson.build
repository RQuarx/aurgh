project('aurgh', 'cpp',
        default_options: [ 'cpp_std=c++23', 'warning_level=3' ],
        version: '1.9.2-alpha',
        license: 'GPL-3.0-or-later',
        license_files: [ 'COPYING' ])

cxx_args = [
    '-DAPP_NAME="'    + meson.project_name()    + '"',
    '-DAPP_VERSION="' + meson.project_version() + '"'
]

add_project_dependencies([
    dependency('fontconfig'),
    dependency('gtkmm-3.0'),
    dependency('libcurl'),
    dependency('jsoncpp'),
    dependency('libalpm')
], language: 'cpp')

include = include_directories('include', '.')

subdir('data')
subdir('src')

install_subdir('data', install_dir: '/usr/share/aurgh')
executable('aurgh', source_files,
           include_directories: include,
           cpp_args: cxx_args,
           install: true)