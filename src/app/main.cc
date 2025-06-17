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

#include "package/client.hh"
#include "package/tab.hh"
#include "arg_parser.hh"
#include "config.hh"
#include "logger.hh"
#include "data.hh"


static const str     APP_ID         = "org.rquarx.aur-graphical-helper";
static const int32_t CURL_INIT_FLAG = CURL_GLOBAL_ALL | CURL_VERSION_THREADSAFE;


namespace {
    class AppWindow : public Gtk::Window
    {
    public:
        AppWindow()
        {
            str title = data::arg_parser->get_option("title");
            if (title.empty()) {
                title = (*data::config->get_config())
                    ["app"]["default-title"].asString();
            }

            set_title(title);
            auto *pkg_tab = Gtk::make_managed<pkg::Tab>();

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
    data::arg_parser = std::make_shared<ArgParser>(argc, argv);

    data::arg_parser->add_options(
        ArgInput({ "-l", "--log" },    "Shows or outputs the log", "path,int"),
        ArgInput({ "-t", "--title" },  "Changes the window title", "str"),
        ArgInput({ "-c", "--config" }, "Specify a config path", "path")
    ).add_flags(
        ArgInput({ "-V", "--version" }, "Prints the program version")
    ).parse();

    if (data::arg_parser->get_flag("version")) {
        std::println(
            "{} {}\n"
            "├─jsoncpp - {}\n"
            "├─libcurl - {}\n"
            "├─libalpm - {}\n"
            "├─glibmm  - {}.{}.{}\n"
            "╰─gtkmm   - {}.{}.{}",
            APP_NAME, APP_VERSION,
            JSONCPP_VERSION_STRING,
            curl_version_info(CURLVERSION_NOW)->version,
            alpm_version(),
            GLIBMM_MAJOR_VERSION, GLIBMM_MINOR_VERSION, GLIBMM_MICRO_VERSION,
            GTKMM_MAJOR_VERSION, GTKMM_MINOR_VERSION, GTKMM_MICRO_VERSION
        );
        return EXIT_SUCCESS;
    }

    data::app            = Gtk::Application::create(APP_ID);
    data::logger         = std::make_shared<Logger>();
    data::config         = std::make_shared<Config>();
    data::pkg_client     = std::make_shared<pkg::Client>();
    data::installed_pkgs = std::make_shared<uset<str>>();

    data::logger->log(Logger::Debug, "Initialising curl");
    if (curl_global_init(CURL_INIT_FLAG) != 0) {
        data::logger->log(Logger::Error, "Failed to init curl");
    }

#if GTK4
    return data::app->make_window_and_run<AppWindow>(0, nullptr);
#else
    AppWindow window;
    return data::app->run(window);
#endif
}
