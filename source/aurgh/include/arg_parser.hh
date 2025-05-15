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

#pragma once
#ifndef ARG_PARSER_HH__
#define ARG_PARSER_HH__

#include <unordered_map>
#include <cstdint>
#include <format>
#include <string>
#include <vector>
#include <print>


/**
 * @class ArgParser
 * @brief Parses command-line arguments in a flexible and POSIX-compliant way.
 *
 * This class provides advanced parsing of short and long arguments:
 *
 * - '-Syu' is parsed as '-S', '-y', and '-u'
 * - '-Iinc', '-I=inc', and '-I inc' are all parsed correctly
 * - '--include inc' and '--include=inc' are supported
 */
class ArgParser
{
    using arg_pair = std::pair<std::string_view, std::string_view>;
public:
    /**
     * @brief Constructs an ArgParser instance.
     *
     * @param argc Argument count.
     * @param argv Array of C-style argument strings.
     *
     * The constructor parses the arguments and classifies them based on
     * their prefix:
     * - '-' is treated as short-form
     * - '--' is treated as long-form
     * - unprefixed values are treated as options/parameters
     */
    ArgParser(int32_t argc, char **argv);


    /**
     * @brief Checks whether a given argument is present.
     *
     * @param arg A pair of short and long argument forms.
     * @return True if found, false otherwise.
     */
    [[nodiscard]]
    auto find_arg(arg_pair arg) -> bool;


    /**
     * @brief Retrieves the value of a given argument if it exists.
     *
     * @param option Output string that receives the value.
     * @param arg A pair of short and long argument forms.
     * @return True if the option is found and extracted, false otherwise.
     */
    [[nodiscard]]
    auto option_arg(std::string &option, arg_pair arg) -> bool;


    /**
     * @brief Returns the last non-prefixed argument, if any.
     *
     * @return A string representing the last trailing argument.
     */
    [[nodiscard]]
    auto back() -> std::string;


    /**
     * @brief Prints a help message.
     *
     * @param stream Output stream (e.g., stdout or stderr).
     */
    void print_help_message(FILE *stream);


    /**
     * @brief Prints a version message.
     *
     * @param stream Output stream (e.g., stdout or stderr).
     */
    void print_version_message(FILE *stream);

private:
    /**
     * @enum Type
     * @brief Represents argument type: short or long.
     */
    enum Type : uint8_t
    {
        Short = 0, /* -V, -h */
        Long  = 1, /* --version, --help */
    };

    static const constexpr std::string_view bold  = "\033[1m";
    static const constexpr std::string_view line  = "\033[4m";
    static const constexpr std::string_view reset = "\033[0m";

    std::unordered_map<Type, std::vector<std::pair<bool, std::string_view>>>
                     m_arg_list;
    std::string_view m_bin_path;
    FILE            *m_print_stream{};


    /**
     * @brief Looks for a short-form option and retrieves its parameter.
     *
     * @param option Output string for the found value.
     * @param short_arg Short-form argument to search for.
     * @return True if the value is found and assigned.
     */
    auto find_option_short(
        std::string     &option,
        std::string_view short_arg
    ) -> bool;


    /**
     * @brief Looks for a long-form option and retrieves its parameter.
     *
     * @param option Output string for the found value.
     * @param long_arg Long-form argument to search for.
     * @return True if the value is found and assigned.
     */
    auto find_option_long(
        std::string     &option,
        std::string_view long_arg
    ) -> bool;


    template<typename... T_Args>
    auto print_arg(std::string_view fmt, T_Args&&... args)
    {
        std::string msg = std::vformat(fmt, std::make_format_args(args...));
        std::println(m_print_stream, "{}", msg);
    }


    /**
     * @brief Strips leading dashes from an argument pair.
     *
     * @param input_arg The raw short and long argument views.
     * @return A cleaned pair with prefixes removed.
     */
    static auto clean_input_arg(const arg_pair &input_arg) -> arg_pair;
};

#endif /* arg_parser.hh */