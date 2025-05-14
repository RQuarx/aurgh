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

#include <algorithm>
#include <print>

#include <gtkmmconfig.h>

#include "arg_parser.hh"
#include "utils.hh"


ArgParser::ArgParser(int32_t argc, char **argv) :
    m_bin_path(argv[0])
{
    Type previous_type = Short;
    for (int32_t i = 1; i < argc; i++) {
        std::string_view arg = argv[i];

        if (arg.starts_with("--")) {
            m_arg_list[Long].emplace_back(false, arg.substr(2));
            previous_type = Long;
            continue;
        }

        if (arg.starts_with('-')) {
            m_arg_list[Short].emplace_back(false, arg.substr(1));
            previous_type = Short;
            continue;
        }

        m_arg_list[previous_type].emplace_back(true, arg);
    }

    /* A way to make main cleaner. */

    if (find_arg({ "-h", "--help" })) {
        print_help_message(stdout);
        exit(EXIT_SUCCESS);
    }

    if (find_arg({ "-V", "--version" })) {
        print_version_message(stdout);
        exit(EXIT_SUCCESS);
    }
}


auto
ArgParser::clean_input_arg(const arg_pair &input_arg) -> arg_pair
{
    arg_pair cleaned_arg;

    /* Remove a dash from short args (-X -> X) */
    cleaned_arg.first = input_arg.first.starts_with('-')
        ? input_arg.first.substr(1)
        : input_arg.first;

    /* Remove double-dash from long args (--arg -> arg) */
    cleaned_arg.second = input_arg.second.starts_with("--")
        ? input_arg.second.substr(2)
        : input_arg.second;

    return cleaned_arg;
}


auto
ArgParser::find_arg(arg_pair arg) -> bool
{
    arg = clean_input_arg(arg);

    if (m_arg_list.contains(Short)) {
        for (const auto &a : m_arg_list.at(Short)) {
            if (!a.first && a.second.contains(arg.first)) return true;
        }
    }

    if (!m_arg_list.contains(Long)) return false;
    return std::ranges::any_of(m_arg_list.at(Long), [&arg](const auto &a){
        return !a.first && a.second == arg.second;
    });
}


auto
ArgParser::option_arg(std::string &option, arg_pair arg) -> bool
{
    arg = clean_input_arg(arg);

    if (!find_option_short(option, arg.first)) {
        return find_option_long(option, arg.second);
    }
    return true;
}


auto
ArgParser::find_option_short(
    std::string &option, std::string_view short_arg) -> bool
{
    if (!m_arg_list.contains(Short)) return false;

    const size_t args_amount = m_arg_list.at(Short).size();
    const char arg           = short_arg.at(0);

    for (size_t i = 0; i < args_amount; i++) {
        const auto &a = m_arg_list.at(Short).at(i);
        if (a.first) continue;

        /* Case: -X=value */
        if (a.second.contains('=')) {
            auto arg_and_option = str::split(a.second, a.second.find('='));

            if (arg_and_option.front().back() == arg) {
                option = arg_and_option.back();
                return true;
            }
        }

        /* Case: -Xvalue */
        if (a.second.length() > 1) {
            if (a.second.front() == arg) {
                option = a.second.substr(1);
                return true;
            }
        }

        /* Case: -X value */
        for (const auto &c : a.second) {
            if (c == arg && i + 1 < args_amount) {
                const auto &next_arg = m_arg_list.at(Short).at(i + 1);
                if (next_arg.first) {
                    option = next_arg.second;
                    return true;
                }
            }
        }
    }

    option.clear();
    return false;
}


auto
ArgParser::find_option_long(
    std::string &option, std::string_view long_arg) -> bool
{
    if (!m_arg_list.contains(Long)) return false;

    const size_t args_amount = m_arg_list.at(Long).size();

    for (size_t i = 0; i < args_amount; i++) {
        const auto &a = m_arg_list.at(Long).at(i);
        if (a.first) continue;

        /* Case: --arg=value */
        if (a.second.contains('=')) {
            size_t eq_pos = a.second.find('=');

            arg_pair split_arg = {
                a.second.substr(0, eq_pos), a.second.substr(eq_pos + 1)
            };

            if (split_arg.first == long_arg) {
                option = split_arg.second;
                return true;
            }
        }

        /* Case: --arg value */
        if (a.second == long_arg && i + 1 < args_amount) {
            const auto &next_arg = m_arg_list.at(Long).at(i + 1);

            if (next_arg.first) {
                option = next_arg.second;
                return true;
            }
        }
    }

    option.clear();
    return false;
}


auto
ArgParser::back() -> std::string
{
    for (const auto &t : { Short, Long }) {
        if (!m_arg_list.contains(t)) continue;
        auto back = m_arg_list.at(t).back();
        if (back.first) return std::string(back.second);
    }

    return "";
}


void
ArgParser::print_help_message(FILE *stream)
{
    std::vector<std::array<std::string, 3>> options{{
        {{ "-h,--help", "", "Prints the help message." }},
        {{ "-V,--version", "", "Prints the current version." }},
        {{ "-l,--log", "{file,level}", "Shows or outputs the logs." }},
        {{ "-t,--title", "{title}", "Changes the window title." }},
        {{ "-c,--config", "{path}", "Specify a non-default config path." }},
        {{ "-u,--ui", "{path}", "Specify a non-default ui-file path." }},
        {{ "   --no-cache", "", "Forces the program to not cache." }},
    }};

    std::println(stream, "\033[1m\033[4mUsage:\033[0m");
    std::println(stream, "\t\033[1m{}\033[0m <options {{params}}>\n", m_bin_path);
    std::println(stream, "\033[1m\033[4mOptions:\033[0m");

    for (const auto &m : options) {
        std::println(
            stream,
            "\t\033[1m{:<15}\033[0m{:<15}{}",
            m.at(0), m.at(1), m.at(2)
        );
    }

    std::println("\n\033[1m\033[4mCopyright (C) 2025 RQuarx\033[0m\n");
    std::println("This program may be freely redistributed under");
    std::println("the terms of the GNU General Public License.");
}


void
ArgParser::print_version_message(FILE *stream)
{
    std::println(
        stream,
        "\033[1m\033[4m{} {}\033[0m using \033[1m\033[4mGTK {}.{}\033[0m",
        APP_NAME, APP_VERSION,
        GTKMM_MAJOR_VERSION, GTKMM_MINOR_VERSION
    );
    std::println(stream, "\n\033[1m\033[4mCopyright (C) 2025 RQuarx\033[0m\n");
    std::println(stream, "This program may be freely redistributed under");
    std::println(stream, "the terms of the GNU General Public License.");
}