aurgh_src = files(
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


executable(
    'aurgh',
    aurgh_src,
    include_directories: include_directories(
        'include/package',
        'include',
    )
)