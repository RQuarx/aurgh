project(
    'aurgh',
    'cpp',
    version: '0.4.0',
    default_options: [
        'cpp_std=c++23',
        'optimization=3',
        'warning_level=3'
    ],
    license: 'GPL-3.0-or-later',
    license_files: 'COPYING',
    meson_version: '>=1.8.1',
)

gtk_version     = get_option('gtk-version')
pretty_logging = get_option('pretty-logging')

deps = [
    'libcurl',
    'jsoncpp',
    'libalpm',
]

if gtk_version == 4
    deps += [
        'gtkmm-4.0',
        'glibmm-2.68',
    ]
elif gtk_version == 3
    deps += [
        'gtkmm-3.0',
        'glibmm-2.4',
    ]
else
    error('GTK must be of version 3 or 4')
endif

foreach dep : deps
    add_project_dependencies(
        dependency(dep), language: 'cpp'
    )
endforeach

add_project_arguments([
    '-DGDK_DISABLE_DEPRECATED',
    '-DGTK_DISABLE_DEPRECATED',
    '-DAPP_VERSION="@0@"'.format(meson.project_version()),
    '-DAPP_NAME="@0@"'.format(meson.project_name()),
    '-DPRETTY_LOGGING=@0@'.format(pretty_logging),
    '-DGTK4=@0@'.format(gtk_version == 4),
    '-DGTK3=@0@'.format(gtk_version == 3),
], language: 'cpp')

app_files = files(
    'src/app/package/card.cc',
    'src/app/package/client.cc',
    'src/app/package/tab.cc',
    'src/app/config.cc',
    'src/app/dialog.cc',
)

cli_files = files(
    'src/cli/alpm.cc',
)

sources = files(
    'src/arg_parser.cc',
    'src/logger.cc',
    'src/main.cc',
    'src/utils.cc',
) + cli_files + app_files

executable('aurgh', sources,
           include_directories: include_directories('include',
                                                    'include/app',
                                                    'include/cli'))

