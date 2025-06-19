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
#ifndef __ARG_PARSER__HH
#define __ARG_PARSER__HH

#include <unordered_map>
#include <utility>
#include <variant>
#include <format>
#include <string>
#include <vector>
#include "types.hh"

using sv       = str_view;
using arg_pair = pair<str, str>;


struct ArgInput {
    arg_pair arg;
    str
        description,
        param_description;

    ArgInput(arg_pair p_arg, str desc, str param_desc = "") :
        arg(p_arg),
        description(std::move(desc)),
        param_description(std::move(param_desc))
    {}
};


template <>
struct std::formatter<ArgInput> : std::formatter<str> {
    auto
    format(const ArgInput& input, std::format_context& ctx) const
    {
        str
            flag_str = std::format("{},{}", input.arg.first, input.arg.second),
            result   = std::format("\t\033[1m{:<20}\033[0m{:<15}{:<15}",
                                   flag_str,
                                   input.param_description,
                                   input.description);
        return std::formatter<str>::format(result, ctx);
    }
};


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
public:
    ArgParser(i32 argc, char **argv);

    /**
     * @brief Parses the command-line arguments.
     *
     * This must be called after defining all expected arguments using add_*().
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
    auto get_flag(const str &name) -> bool;


    /**
     * @brief Gets the value of an option argument.
     *
     * @param name Name of the flag (stripped long argument form).
     * @return The value passed to the option, or an empty string.
     *
     * @note Return value must not be ignored.
     */
    [[nodiscard("Ignoring return value of an argument parser!")]]
    auto get_option(const str &name) -> str;


    /**
     * @brief Adds one or more argument definitions to the parser.
     *
     * Appends each provided option to the internal argument list. Validates
     * that each option has a long-form flag and registers its name with a
     * default parameter.
     *
     * @tparam T_Args Argument types (typically ArgInput).
     * @param options One or more argument definitions to add.
     * @return Reference to this ArgParser (for chaining).
     *
     * @throws std::invalid_argument If any long-form flag is missing.
     *
     * @details
     * For each new argument:
     * - Ensures the long flag (e.g. --log) is not empty.
     * - Extracts the argument name using `clean_arg`.
     * - Initializes a placeholder parameter using `option_arg`.
     * - Registers the name in the internal argument map.
     *
     * The [[nodiscard]] attribute warns if the returned reference is
     * ignored, encouraging chaining ending with `parse()`.
     */
    template<typename... T_Args>
    [[nodiscard("add_* chain not ended in a parse() member!")]]
    auto add_options(T_Args &&...options) -> ArgParser&
    {
        usize i = m_defined_args.size();
        (m_defined_args.emplace_back(std::forward<T_Args>(options)), ...);

        for (; i < m_defined_args.size(); i++) {
            const ArgInput &input = m_defined_args.at(i);
            arg_pair        clean = clean_arg(input.arg);

            if (input.arg.second.empty()) {
                throw std::invalid_argument("Long arg cannot be empty!");
            }

            str param;

            bool _         = option_arg(param, input.arg);
            m_user_arg.insert_or_assign(clean.second, param);
        }

        return *this;
    }


    /**
     * @brief Adds one or more flags to the argument parser.
     *
     * Appends each flag to the internal argument list and processes
     * special flags such as "help" and "version". Validates that all
     * flags have non-empty long forms and registers them.
     *
     * @tparam T_Args Variadic template for argument definitions.
     * @param options One or more flags to add.
     * @return Reference to this ArgParser for method chaining.
     *
     * @throws std::invalid_argument If any flag has an empty long name.
     *
     * @details
     * For each added flag:
     * - Cleans the argument name using `clean_arg`.
     * - Sets override flags for "help" or "version".
     * - Throws if the long form is empty.
     * - Inserts or updates the flag in the user argument map using
     *   `find_arg`.
     *
     * The [[nodiscard]] attribute warns if the returned reference is
     * ignored, encouraging chaining ending with `parse()`.
     */
    template<typename... T_Args>
    [[nodiscard("add_* chain not ended in a parse() member!")]]
    auto add_flags(T_Args &&...options) -> ArgParser&
    {
        usize i = m_defined_args.size();
        (m_defined_args.emplace_back(std::forward<T_Args>(options)), ...);

        for (; i < m_defined_args.size(); i++) {
            const ArgInput &input = m_defined_args.at(i);
            arg_pair        clean = clean_arg(input.arg);

            if (clean.second == "version") m_override_version = true;
            if (clean.second == "help")    m_override_help    = true;

            if (input.arg.second.empty()) {
                throw std::invalid_argument("Long arg cannot be empty!");
            }

            m_user_arg.insert_or_assign(clean.second, find_arg(clean));
        }

        return *this;
    }

private:
    enum Type : u8
    {
        Short = 0,
        Long  = 1
    };

    using arg_t      = umap<Type, vec<pair<bool, str>>>;
    using user_arg_t = umap<str, std::variant<bool, str>>;

    arg_t       m_arg_list;
    str         m_bin_path;
    user_arg_t  m_user_arg;

    vec<ArgInput> m_defined_args;

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
    auto option_arg(str &param, arg_pair arg) -> bool;


    /**
     * @brief Checks if a short option matches input.
     *
     * @param option The input argument.
     * @param short_arg Expected short option name.
     * @return True if matched.
     */
    [[nodiscard]]
    auto find_option_short(str &option, sv short_arg) -> bool;


    /**
     * @brief Checks if a long option matches input.
     *
     * @param option The input argument.
     * @param long_arg Expected long option name.
     * @return True if matched.
     */
    [[nodiscard]]
    auto find_option_long(str &option, sv long_arg) -> bool;


    /**
     * @brief Splits a string at a given position.
     *
     * @param str The input string.
     * @param pos The index to split at.
     * @return A pair of substrings.
     */
    [[nodiscard]]
    static auto split_str(sv str, usize pos) -> arg_pair;
};

#endif /* __ARG_PARSER__HH */