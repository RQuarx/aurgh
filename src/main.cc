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

/* ! IMPORTANT APPLICATION DATA ! */
static const std::string_view APP_VERSION = "0.0.12"; /* Bump minor on installation feature */


auto
main(int32_t argc, char **argv) -> int32_t
{
    auto app = Gtk::Application::create("org.rquarx.aur-graphical-helper");
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
    AUR::Client  aur_client(&logger, "");
    Gtk::Window window(Gtk::WINDOW_TOPLEVEL);

    // auto proc = Process("git", { "clone", "https://github.com/git/git" }, &logger);

    if (curl_global_init(CURL_GLOBAL_ALL | CURL_VERSION_THREADSAFE) != 0) {
        logger.log(Logger::Error, "Failed to init curl");
        return EXIT_FAILURE;
    }

    auto *package_tab = Gtk::make_managed<pkg::Tab>(&aur_client, &logger);

    window.set_title("AUR Graphical Helper");
    window.add(*package_tab);
    window.show_all();

    /* Since in GTK4, GTK doesnt requires an argv to run.
       Therefore, it would be best practice to make it work
       like that in GTK3
    */
    argc = 0;

    // logger.log(Logger::Debug, "Return: {}", proc.is_done());

    return app->run(window, argc, argv);
}