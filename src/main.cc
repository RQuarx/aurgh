/**
 * @file main.cc
 *
 * This file is part of aurgh
 *
 * aurgh is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * aurgh is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with aurgh. If not, see <https://www.gnu.org/licenses/>.
 */

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
static const std::string_view APP_VERSION = "0.0.4";


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
            std::string_view arg = argv[i];

            if (arg.starts_with("--log")
                || (arg.starts_with('-') && arg.contains('l'))
            ) {
                for (int32_t j = i; j < argc - 1; j++) {
                    argv[j] = argv[j + 1];
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