project('aurgh', 'cpp',
        default_options: [ 'cpp_std=c++23', 'warning_level=3' ],
        version: '1.7.1-alpha',
        license: 'GPL-3.0-or-later')

add_project_arguments([
    '-DAPP_NAME="@0@"'.format(meson.project_name()),
    '-DAPP_VERSION="@0@"'.format(meson.project_version())
], language: 'cpp')

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
    
)

main = files(
    'src/log.cc',
    'src/main.cc',
    'src/utils.cc',
) + app + cli

subdir('data')

install_subdir('data', install_dir: '/usr/share/aurgh')

executable('aurgh', main,
           include_directories: include_directories('include', '.'),
           install: true,
           install_dir: '/usr/bin')