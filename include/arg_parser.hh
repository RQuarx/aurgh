/**
 * @file arg_parser.hh
 *
 * This file is part of AURGH
 *
 * AURGH is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * AURGH is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#ifndef ARG_PARSE_HH__
#define ARG_PARSE_HH__

#include <unordered_map>
#include <cstdint>
#include <string>
#include <vector>

using arg_pair = std::pair<std::string_view, std::string_view>;


/**
 * @class ArgParser
 * @brief A class made to parse command line argument in a perfect way.
 *
 * This class provides an advanced way to detect command line arguments,
 * an example:
 *
 * - '-Syu' would give out '-S' '-y' and '-u'
 *
 * - '-Iinc', '-I=inc', '-I inc' are all valid
 *
 * - '--include inc', '--include=inc' are all also a valid option argument
 */
class ArgParser
{
public:
    /**
     * @brief Constructs a ArgParser class.
     * @param argc The number of given command line arguments
     * @param argv An array of C-style string representing the arguments
     *
     * The constructor will parses the arguments based on the line-prefixes,
     * it'll parse:
     *
     * - '-' as a short-form argument
     *
     * - '--' as a short-form argument
     *
     * - an argument with no line-prefix to be an option to the previous type's argument
     */
    ArgParser(int32_t argc, char **argv);

    /**
     * @brief Checks if the specified argument exists in the command line arguments
     * @param arg A pair object containing 2 string_views, which represent the short and long form of the argument
     * @returns true if a match is found in the command line arguments; otherwise, false
     */
    auto find_arg(arg_pair arg) -> bool;

    /**
     * @brief Searches for and returns the value of a specified argument.
     * @param option A reference to a string that will be filled with the value of the argument if found.
     * @param arg A pair object containing both the short and long form of the argument.
     * @returns true if a match is found and the option is filled; otherwise, false.
     */
    auto option_arg(std::string &option, arg_pair arg) -> bool;

    /**
     * @brief Returns the last option arg
     */
    auto back() -> std::string;

    /**
     * @brief Prints a help message, and exist the program after it
     * @param stream An output stream the help message will be printed to
     * @param operation_type An OperationType enum which the function use to pick the help message to print, defaults to None
     */
    void print_help_message(FILE *stream);

private:
    /**
     * @enum Type
     * @brief An enum representing the argument type
     */
    enum Type : uint8_t {
        Short = 0,
        Long = 1,
    };

    std::unordered_map<Type, std::vector<std::pair<bool, std::string_view>>>
                     m_arg_list;
    std::string_view m_bin_path;

    /**
     * @brief Removes line-prefixes from the arg_input
     */
    static auto clean_input_arg(const arg_pair &input_arg) -> arg_pair;

    /**
     * @brief Searches for command-line argument parameter for short argument
     * @param option A reference to a string that will be filled with the value of the argument if found.
     * @param short_arg A string_view containing the short argument
     * @returns true if a match is found and the option is filled; otherwise, false.
     */
    auto find_option_short(std::string &option, std::string_view short_arg) -> bool;

    /**
     * @brief Searches for command-line argument parameter for long argument
     * @param option A reference to a string that will be filled with the value of the argument if found.
     * @param long_arg A string_view containing the long argument
     * @returns true if a match is found and the option is filled; otherwise, false.
     */
    auto find_option_long(std::string &option, std::string_view long_arg) -> bool;
};

#endif /* arg_parse.hh */