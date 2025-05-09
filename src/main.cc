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

#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <curl/curl.h>

#include "package/tab.hh"
#include "arg_parser.hh"
#include "aur_client.hh"
#include "process.hh"
#include "logger.hh"

/* ! IMPORTANT APPLICATION DATA ! */

static const std::string_view DEFAULT_TITLE = "AUR Graphical Helper";
static const std::string APP_ID             = "org.rquarx.aur-graphical-helper";

/* Bump minor on installation feature */
static const std::string_view APP_VERSION = "0.0.13";


namespace {
    class Window : public Gtk::Window
    {
    public:
        explicit Window(ArgParser &arg_parser, pkg::Tab &tab)
        {
            std::string title;
            if (!arg_parser.option_arg(title, { "-t", "--title" })) {
                title = DEFAULT_TITLE;
            }

            set_child(tab);
            set_title(title);
        }
    };
}  /* anonymous namespace */


auto
main(int32_t argc, char **argv) -> int32_t
{
    auto app = Gtk::Application::create(APP_ID);
    ArgParser  arg_parser(argc, argv);

    if (arg_parser.find_arg({ "-h", "--help" })) {
        arg_parser.print_help_message(stdout);
        return EXIT_SUCCESS;
    }

    if (arg_parser.find_arg({ "-v", "--version" })) {
        std::println("AURGH-{}", APP_VERSION);
        return EXIT_SUCCESS;
    }

    Logger      logger(arg_parser);
    AUR::Client aur_client(&logger, "");
    // Gtk::Window window;
    pkg::Tab    tab(&aur_client, &logger);

    if (curl_global_init(CURL_GLOBAL_ALL | CURL_VERSION_THREADSAFE) != 0) {
        logger.log(Logger::Error, "Failed to init curl");
        return EXIT_FAILURE;
    }

    // std::string title;
    // if (!arg_parser.option_arg(title, { "-t", "--title" })) {
    //     title = DEFAULT_TITLE;
    // }

    // window.set_child(tab);
    // window.set_title(title);

    return app->make_window_and_run<Window>(0, nullptr, arg_parser, tab);
}