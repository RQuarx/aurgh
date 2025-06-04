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
#include "config.hh"
#include "logger.hh"
#include "tab.hh"

using const_str = const std::string;

static const_str APP_ID = "org.rquarx.aur-graphical-helper";
static const int32_t CURL_INIT_FLAG = CURL_GLOBAL_ALL | CURL_VERSION_THREADSAFE;


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
            std::string title = arg_parser->get_option("title");
            if (title.empty()) {
                title = (*config->get_config())["app"]["default-title"].asString();
            }

            set_title(title);

            auto *pkg_tab = Gtk::make_managed<pkg::Tab>(
                aur_client, logger, config, arg_parser
            );

#if GTK4
            set_child(*pkg_tab);
#else
            add(*pkg_tab);
#endif
        }
    };
} /* anonymous namespace */


auto
main(int32_t argc, char **argv) -> int32_t
{
    auto arg_parser = std::make_shared<ArgParser>(argc, argv);

    arg_parser
        ->add_flag( { "-V", "--version" }, "Prints the program version")
        .add_option({ "-l", "--log" },     "Shows or outputs the log", "path,int")
        .add_option({ "-t", "--title" },   "Changes the window title", "str")
        .add_option({ "-c", "--config" },  "Specify a config path", "path")
        .parse();

    if (arg_parser->get_flag("version")) {
        std::println(
            "{} {}\n├─libalpm - {}\n╰─gtkmm   - {}.{}.{}",
            APP_NAME, APP_VERSION,
            alpm_version(),
            GTKMM_MAJOR_VERSION, GTKMM_MINOR_VERSION, GTKMM_MICRO_VERSION
        );
        return EXIT_SUCCESS;
    }

    auto app        = Gtk::Application::create(APP_ID);
    auto logger     = std::make_shared<Logger>     (arg_parser);
    auto config     = std::make_shared<Config>     (logger, arg_parser);
    auto aur_client = std::make_shared<AUR::Client>(logger, arg_parser, config);

    logger->log(
        Logger::Debug,
        "Initialising curl with flag: CURL_GLOBAL_ALL, CURL_VERSION_THREADSAFE"
    );
    if (curl_global_init(CURL_INIT_FLAG) != 0) {
        logger->log(Logger::Error, "Failed to init curl");
        return EXIT_FAILURE;
    }

#if GTK4
    return app->make_window_and_run<AppWindow>(
        0, nullptr, aur_client, logger, config, arg_parser
    );
#else
    AppWindow window(aur_client, logger, config, arg_parser);
    return app->run(window, 0, nullptr);
#endif
}
