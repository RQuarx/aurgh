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
#include <string_view>
#include <variant>
#include <cstdint>
#include <string>
#include <vector>


/**
 * @class ArgParser
 * @brief A command-line argument parser supporting short and long options.
 *
 * This class allows registration and parsing of command-line arguments
 * with support for flags (booleans) and options (with values).
 * Arguments can be registered with short (e.g., "-f")
 * and long (e.g., "--file") forms.
 */
class ArgParser
{
    using sv       = std::string_view;
    using str      = std::string;
    using arg_pair = std::pair<str, str>;
public:
    ArgParser(int32_t argc, char **argv);

    /**
     * @brief Registers a boolean flag argument.
     *
     * @param arg Pair of short and long option names (e.g., "-v", "--verbose").
     * @param description Help message for the argument.
     * @return Reference to the parser instance (for method chaining).
     */
    [[nodiscard("add* chain not ended in a parse() member!")]]
    auto add_flag(
        arg_pair           arg,
        const std::string &description
    ) -> ArgParser&;

    /**
     * @brief Registers an option that expects a value.
     *
     * @param arg Pair of short and long option names (e.g., "-f", "--file").
     * @param description Help message for the option.
     * @param param_name Name of the parameter shown in help output.
     * @return Reference to the parser instance (for method chaining).
     */
    [[nodiscard("add* chain not ended in a parse() member!")]]
    auto add_option(
        arg_pair           arg,
        const std::string &description,
        const std::string &param_name = ""
    ) -> ArgParser&;


    /**
     * @brief Parses the command-line arguments.
     *
     * This must be called after defining all expected arguments using add_arg().
     * It processes the actual argv values and stores matched flags and options.
     */
    void parse();


    /**
     * @brief Gets the value of a boolean flag argument.
     *
     * @param name Name of the flag (stripped long argument form).
     * @return True if the flag was set; false otherwise.
     *
     * @note Return value must not be ignored.
     */
    [[nodiscard("Ignoring return value of an argument parser!")]]
    auto get_flag(const std::string &name) -> bool;


    /**
     * @brief Gets the value of an option argument.
     *
     * @param name Name of the flag (stripped long argument form).
     * @return The value passed to the option, or an empty string.
     *
     * @note Return value must not be ignored.
     */
    [[nodiscard("Ignoring return value of an argument parser!")]]
    auto get_option(const std::string &name) -> std::string;

private:
    enum Type : uint8_t
    {
        Short = 0,
        Long  = 1
    };

    template<typename Key, typename Tp>
    using umap          = std::unordered_map<Key, Tp>;
    using arg_cont      = umap<Type, std::vector<std::pair<bool, str>>>;
    using user_arg_cont = umap<str, std::variant<bool, str>>;
    using help_msg_cont = umap<str, std::array<str, 3>>;

    arg_cont      m_arg_list;
    std::string   m_bin_path;
    user_arg_cont m_user_arg;
    help_msg_cont m_help_msg;

    /* Flags */
    bool m_override_help;
    bool m_override_version;


    /**
     * @brief Removes leading dashes from argument names.
     *
     * @param arg A pair of short and long argument names.
     * @return A cleaned argument pair with dashes removed.
     */
    [[nodiscard]]
    static auto clean_arg(const arg_pair &arg) -> arg_pair;


    /**
     * @brief Checks if the argument was provided by the user.
     *
     * @param arg The argument pair to search for.
     * @return True if found; false otherwise.
     */
    [[nodiscard]]
    auto find_arg(arg_pair arg) -> bool;


    /**
     * @brief Matches and assigns the value of an option argument.
     *
     * @param param The string value passed to the argument.
     * @param arg   The corresponding argument pair.
     * @return True if the match was successful; false otherwise.
     */
    [[nodiscard]]
    auto option_arg(std::string &param, arg_pair arg) -> bool;


    /**
     * @brief Checks if a short option matches input.
     *
     * @param option The input argument.
     * @param short_arg Expected short option name.
     * @return True if matched.
     */
    [[nodiscard]]
    auto find_option_short(std::string &option, sv short_arg) -> bool;


    /**
     * @brief Checks if a long option matches input.
     *
     * @param option The input argument.
     * @param long_arg Expected long option name.
     * @return True if matched.
     */
    [[nodiscard]]
    auto find_option_long(std::string &option, sv long_arg) -> bool;


    /**
     * @brief Splits a string at a given position.
     *
     * @param str The input string.
     * @param pos The index to split at.
     * @return A pair of substrings.
     */
    [[nodiscard]]
    static auto split_str(sv str, size_t pos) -> arg_pair;
};

#endif /* arg_parser.hh */