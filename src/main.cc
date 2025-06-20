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

#include <filesystem>

#include <gtkmm/application.h>
#include <gtkmm/builder.h>
#include <gtkmm/window.h>
#include <curl/curl.h>
#include <json/reader.h>

#include "package/client.hh"
#include "package/tab.hh"
#include "arg_parser.hh"
#include "config.hh"
#include "logger.hh"
#include "alpm.hh"
#include "data.hh"


static constexpr auto APP_ID = "org.rquarx.aur-graphical-helper";


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


void
print_version(bool is_gui)
{
    str end = is_gui
    ? std::format(
        "├─libalpm - {}\n"
        "├─glibmm  - {}.{}.{}\n"
        "╰─gtkmm   - {}.{}.{}",
        alpm_version(),
        GLIBMM_MAJOR_VERSION, GLIBMM_MINOR_VERSION, GLIBMM_MICRO_VERSION,
        GTKMM_MAJOR_VERSION, GTKMM_MINOR_VERSION, GTKMM_MICRO_VERSION
    ) : std::format("╰─libalpm - {}\n", alpm_version());
    std::println("{} {}\n"
                 "├─jsoncpp - {}\n"
                 "├─libcurl - {}\n"
                 "{}",
                 APP_NAME, APP_VERSION,
                 JSONCPP_VERSION_STRING,
                 curl_version_info(CURLVERSION_NOW)->version,
                 end);
}


auto
app_mode(i32 p_argc, char **p_argv) -> i32
{
    data::arg_parser = std::make_shared<ArgParser>(p_argc, p_argv);

    data::arg_parser->add_options(
        ArgInput({ "-l", "--log" },    "Shows or outputs the log", "path,int"),
        ArgInput({ "-t", "--title" },  "Changes the window title", "str"),
        ArgInput({ "-c", "--config" }, "Specify a config path", "path")
    ).add_flags(
        ArgInput({ "-V", "--version" }, "Prints the program version")
    ).parse();

    if (data::arg_parser->get_flag("version")) {
        print_version(true);
        return EXIT_SUCCESS;
    }

    data::app            = Gtk::Application::create(APP_ID);
    data::logger         = std::make_shared<Logger>();
    data::config         = std::make_shared<Config>();
    data::pkg_client     = std::make_shared<pkg::Client>();
    data::installed_pkgs = std::make_shared<uset<str>>();

    data::logger->log(Logger::Debug, "Initialising curl");
    if (curl_global_init(CURL_GLOBAL_ALL) != 0) [[unlikely]] {
        data::logger->log(Logger::Error, "Failed to init curl");
    }

#if GTK4
    return data::app->make_window_and_run<AppWindow>(0, nullptr);
#else
    AppWindow window;
    return data::app->run(window);
#endif
}


auto
cli_mode(i32 p_argc, char **p_argv) -> i32
{
    data::arg_parser = std::make_shared<ArgParser>(p_argc, p_argv);

    data::arg_parser->add_options(
        ArgInput({ "-l", "--log" },    "Shows or outputs the log", "path,int"),
        ArgInput({ "-p", "--prefix" }, "Specify the prefix", "path")
    ).add_flags(
        ArgInput({ "-V", "--version" }, "Prints the program version")
    ).parse();

    if (data::arg_parser->get_flag("version")) {
        print_version(false);
        return EXIT_SUCCESS;
    }

    if (data::arg_parser->get_option("prefix").empty()) return EXIT_FAILURE;

    data::logger = std::make_shared<Logger>();

    str
        prefix_path         = data::arg_parser->get_option("prefix"),
        operation_file_path = prefix_path + "/operation.json",
        root_path,
        db_path,
        type;

    json          operation      = Json::objectValue;
    std::ifstream operation_file { operation_file_path };
    operation_file >> operation;
    operation_file.close();

    root_path = operation["root"     ].asString();
    db_path   = operation["db-path"  ].asString();
    type      = operation["operation"].asString();

    alpm_errno_t err = ALPM_ERR_OK;
    Alpm         alpm {root_path, db_path, prefix_path, err};

    using std::filesystem::remove;

    if (err != ALPM_ERR_OK) {
        data::logger->log(Logger::Error,
                            "Failed to initialize alpm: {}",
                            alpm_strerror(err));
        return remove(operation_file_path), 1;
    };

    if (type == "remove") {
        vec<str> pkgs;
        pkgs.reserve(operation["pkgs"].size());

        for (const auto &p : operation["pkgs"]) pkgs.push_back(p.asString());

        bool _ = alpm.remove_packages(pkgs);
        remove(operation_file_path);
        return err;
    }

    if (type == "install") {
        bool _ = alpm.download_and_install_package(operation["pkg"].asString());
        return remove(operation_file_path), 1;
    }

    return remove(operation_file_path), 0;
}
} /* anonymous namespace */


auto
main(i32 p_argc, char **p_argv) -> i32
{ return getuid() == 0 ? cli_mode(p_argc, p_argv) : app_mode(p_argc, p_argv); }
