project('aurgh', 'cpp',
        default_options: [ 'cpp_std=c++23', 'warning_level=3' ],
        license_files:   [ 'COPYING' ],
        license:         'GPL-3.0-or-later',
        version:         '0.1.0',
        meson_version:   '>=1.4.0')

compile_args = [ '-DPROJECT_VERSION="@0@"'.format(meson.project_version()) ]
dependencies = [ dependency('libcurl'),
                 dependency('gtkmm-4.0',     required: true), # Must be provided by the system
                 dependency('nlohmann_json', fallback: [ 'nlohmann_json', 'nlohmann_json_dep' ]),
                 dependency('lyra',          fallback: [ 'Lyra',          'lyra_dep' ]) ]

subdir('ui')
blueprints = custom_target(command: [ find_program('blueprint-compiler'),
                                      'batch-compile',
                                      '--minify',
                                      '@OUTPUT@',
                                      '@CURRENT_SOURCE_DIR@',
                                      '@INPUT@' ],
                           output: '.',
                           input:  blueprint_src)

gnome = import('gnome')

res = gnome.compile_resources('aurgh_resources', 'aurgh.gresources.xml',
                              source_dir:   '.',
                              dependencies: blueprints,
                              c_name:       'aurgh_resources')

subdir('src')
executable(meson.project_name(), [ src, res ],
           install:             true,
           cpp_args:            compile_args,
           dependencies:        dependencies,
           include_directories: include_directories('include'))