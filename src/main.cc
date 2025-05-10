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

#include <gtkmm/window.h>
#include <curl/curl.h>

#include "package/tab.hh"
#include "arg_parser.hh"
#include "aur_client.hh"
#include "process.hh"
#include "logger.hh"

using const_str = const std::string;

/* ! IMPORTANT APPLICATION DATA ! */

static const_str APP_ID               = "org.rquarx.aur-graphical-helper";
static const_str DEFAULT_WINDOW_TITLE = "AUR Graphical Helper";

/* Bump minor on installation feature */
static const_str APP_VERSION = "0.0.14";


auto
main(int32_t argc, char **argv) -> int32_t
{
    auto app        = Gtk::Application::create(APP_ID);
    auto arg_parser = std::make_shared<ArgParser>(argc, argv);

    if (arg_parser->find_arg({ "-h", "--help" })) {
        arg_parser->print_help_message(stdout);
        return EXIT_SUCCESS;
    }

    if (arg_parser->find_arg({ "-v", "--version" })) {
        std::println("AURGH-{}", APP_VERSION);
        return EXIT_SUCCESS;
    }

    auto logger     = std::make_shared<Logger>(arg_parser);
    auto aur_client = std::make_shared<AUR::Client>(logger, "");
    Gtk::Window window(Gtk::WINDOW_TOPLEVEL);

    if (curl_global_init(CURL_GLOBAL_ALL | CURL_VERSION_THREADSAFE) != 0) {
        logger->log(Logger::Error, "Failed to init curl");
        return EXIT_FAILURE;
    }

    auto *package_tab = Gtk::make_managed<pkg::Tab>(aur_client, logger);

    std::string title;
    if (!arg_parser->option_arg(title, { "-t", "--title" })) {
        title = DEFAULT_WINDOW_TITLE;
    }

    window.set_title(title);
    window.add(*package_tab);
    window.show_all();

    return app->run(window, 0, nullptr);
}