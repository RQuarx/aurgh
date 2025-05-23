/**
 * aurgh Copyright (C) 2025 RQuarx
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

#include "arg_parser.hh"
#include "aur_client.hh"
#include "process.hh"
#include "config.hh"
#include "logger.hh"
#include "tab.hh"

using const_str = const std::string;

static const_str APP_ID = "org.rquarx.aur-graphical-helper";


#if GTKMM_MAJOR_VERSION == 4
namespace {
    using std::shared_ptr;


    class AppWindow : public Gtk::Window
    {
    public:
        AppWindow(
            const shared_ptr<AUR::Client> &aur_client,
            const shared_ptr<Logger>      &logger,
            const shared_ptr<Config>      &config,
            const shared_ptr<ArgParser>   &arg_parser
        )
        {
            std::string title;
            if (!arg_parser->option_arg(title, { "-t", "--title" })) {
                title = (*config->get_config())["default-title"].asString();
            }

            set_title(title);
            set_child(
                *Gtk::make_managed<pkg::Tab>(
                    aur_client, logger, config, arg_parser
                )
            );
        }
    };
} /* anonymous namespace */
#endif


auto
main(int32_t argc, char **argv) -> int32_t
{
    auto app        = Gtk::Application::create(APP_ID);
    auto arg_parser = std::make_shared<ArgParser>  (argc, argv);
    auto logger     = std::make_shared<Logger>     (arg_parser);
    auto config     = std::make_shared<Config>     (logger, arg_parser);
    auto aur_client = std::make_shared<AUR::Client>(logger, arg_parser, config);

    if (curl_global_init(CURL_GLOBAL_ALL | CURL_VERSION_THREADSAFE) != 0) {
        logger->log(Logger::Error, "Failed to init curl");
        return EXIT_FAILURE;
    }

#if GTKMM_MAJOR_VERSION == 4
    return app->make_window_and_run<AppWindow>(
        0, nullptr, aur_client, logger, config, arg_parser
    );
#else
    Gtk::Window window(Gtk::WINDOW_TOPLEVEL);

    std::string title;
    if (!arg_parser->option_arg(title, { "-t", "--title" })) {
        title = (*config->get_config())["default-title"].asString();
    }

    window.set_title(title);
    window.add(
        *Gtk::make_managed<pkg::Tab>(aur_client, logger, config, arg_parser)
    );

    return app->run(window, 0, nullptr);
#endif
}