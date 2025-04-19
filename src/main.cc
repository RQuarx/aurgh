#include <cstdint>
#include <gtkmm-3.0/gtkmm.h>
#include <json/value.h>
#include <curl/curl.h>

#include "tabs/packages.hh"
#include "arg_parser.hh"
#include "aur_client.hh"
#include "logger.hh"
#include "window.hh"

static const std::string_view HELP_MSG =
"Usage:\n\taurgh [OPTION…]\n\n" \
"Help Options:\n" \
"\t-h, --help                 Show help options\n" \
"\t--help-all                 Show all help options\n" \
"\t--help-gapplication        Show GApplication options\n" \
"\t--help-gtk                 Show GTK+ Options\n\n" \
"Application Options:\n" \
"\t--display=DISPLAY          X display to use\n" \
"\t-l, --log=LEVEL,FILE       Logs based on the provided level (0-3) and or a file\n" \
"\t-v, --version              Show application version\n";

/* ! IMPORTANT APPLICATION DATA ! */
static const std::string_view APP_VERSION = "0.0.1";


auto
main(int32_t argc, char **argv) -> int32_t
{
    auto app = Gtk::Application::create("org.rquarx.aur-graphical-helper");
    ArgParser  arg_parser(argc, argv);

    if (arg_parser.find_arg({ "-h", "--help" })) {
        std::println("{}", HELP_MSG);
        return EXIT_SUCCESS;
    }

    if (arg_parser.find_arg({ "-v", "--version" })) {
        std::println("AURGH-{}", APP_VERSION);
        return EXIT_SUCCESS;
    }

    Logger logger(arg_parser);

    /* Removes -l, --log arg from argv so gtk doesnt complain */
    if (arg_parser.find_arg({ "-l", "--log" })) {
        for (int32_t i = 0; i < argc; i++) {
            if (
                (argv[i][0] == '-' && argv[i][1] == 'l') ||
                (argv[i][2] == 'l' && argv[i][3] == 'o' && argv[i][4] == 'g')
            ) {
                argv[i] = nullptr;

                for (int32_t j = i; j < argc - 1; j++) {
                    argv[j] = argv[j + 1];
                    argv[j + 1] = nullptr;
                }

                argc--;
                break;
            }
        }
    }

    if (curl_global_init(CURL_GLOBAL_ALL | CURL_VERSION_THREADSAFE) != 0) {
        logger.log(Logger::Error, "Failed to init curl");
        return EXIT_FAILURE;
    }

    AUR_Client aur_client(&logger, "");
    AURWindow  window({ Gtk::make_managed<PackageTab>(&aur_client, &logger), Gtk::make_managed<Gtk::Box>() }, &logger);

    return app->run(window, argc, argv);
}