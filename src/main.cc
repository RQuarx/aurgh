#include <cstdint>
#include <gtkmm-3.0/gtkmm.h>
#include <json/value.h>
#include <curl/curl.h>

#include "tabs/packages.hh"
#include "arg_parser.hh"
#include "aur_client.hh"
#include "logger.hh"
#include "window.hh"

#define HELP_MSG \
"Usage:\n" \
"  aurgh [OPTION…]\n\n" \
"Help Options:\n" \
"  -h, --help                 Show help options\n" \
"  --help-all                 Show all help options\n" \
"  --help-gapplication        Show GApplication options\n" \
"  --help-gtk                 Show GTK+ Options\n\n" \
"Application Options:\n" \
"  --display=DISPLAY          X display to use\n" \
"  -l, --log=LEVEL,FILE       Logs based on the provided level (0-3) and or a file\n"


auto
main(int32_t argc, char **argv) -> int32_t
{
    auto app =
        Gtk::Application::create("org.rquarx.aurgh");

    ArgParser arg_parser(argc, argv);
    Logger logger(arg_parser);

    if (arg_parser.find_arg({ "-l", "--log" })) {
        for (int32_t i = 0; i < argc; i++) {
            std::string arg(argv[i]);
            if (arg.starts_with("-l") || arg.starts_with("--log")) {
                argv[i] = nullptr;
                argc--;
                break;
            }
        }
    }

    if (arg_parser.find_arg({ "-h", "--help" })) {
        std::println("{}", HELP_MSG);
        return 0;
    }

    if (curl_global_init(CURL_GLOBAL_ALL | CURL_VERSION_THREADSAFE) != 0) {
        logger.log(Logger::Error, "Failed to init curl");
        exit(1);
    }
    AUR_Client aur_client(&logger);

    AURWindow window({ Gtk::make_managed<PackageTab>(&aur_client), Gtk::make_managed<Gtk::Box>() }, &logger);

    int32_t return_code = app->run(window, argc, argv);
    return return_code;
    return 0;
}