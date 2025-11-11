project('aurgh', 'cpp',
        default_options: [ 'cpp_std=c++23', 'warning_level=3' ],
        version: '1.9.1-alpha',
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

app = files(
    'src/app/tabs/aur.cc',
    'src/app/tabs/card.cc',
    'src/app/app.cc',
    'src/app/package.cc',
    'src/app/utils.cc',
)

cli = files(
    'src/cli/cli.cc',
)

main = files(
    'src/log.cc',
    'src/main.cc',
    'src/utils.cc',
)

include = include_directories('include', '.')

subdir('data')

install_subdir('data', install_dir: '/usr/share/aurgh')
executable('aurgh', main + app + cli,
           include_directories: include,
           cpp_args: cxx_args,
           install: true)