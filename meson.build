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
    'src/utils.cc'
)

gtkmm_dep = dependency('gtkmm-3.0')
glibmm_dep = dependency('glibmm-2.4')
curl_dep = dependency('libcurl')
jsoncpp_dep = dependency('jsoncpp')
libuv_dep = dependency('libuv')

deps = [gtkmm_dep, glibmm_dep, curl_dep, jsoncpp_dep, libuv_dep]

cpp_args = [
    '-g',
    '-Wall',
    '-Wextra',
    '-DGDK_DISABLE_DEPRECATED',
    '-DGTK_DISABLE_DEPRECATED'
]

executable(
    'aurgh',
    src,
    dependencies: deps,
    cpp_args: cpp_args,
    include_directories: include_directories('include')
)