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
#ifndef __UTILS__HH
#define __UTILS__HH

#include <filesystem>
#include <algorithm>
#include <optional>
#include <format>
#include <string>
#include <vector>
#include <array>

#include <curl/curl.h>

namespace Json { class Value; }
namespace Gtk {
    class Widget;
    class Label;
    class Box;
}
class ArgParser;

namespace fs = std::filesystem;


/**
 * @namespace utils
 * @brief A namespace containing usefull utilities
 */
namespace utils {
    using std::optional;

    static const size_t DEFAULT_COLOR_THRESHOLD = 16UZ;
    static const size_t DEFAULT_BUFFER_SIZE     = 256UZ;


    /**
     * @brief Executes a shell command and captures its output.
     * @param cmd The command string.
     * @param buffer_size Optional buffer size (default: 256).
     * @return Optional pair of output string and exit code.
     */
    auto run_command(
        const std::string &cmd,
        size_t             buffer_size = DEFAULT_BUFFER_SIZE
    ) -> optional<std::pair<std::string, int32_t>>;

    /**
     * @brief Checks if the current terminal supports a given number of colors.
     * @param threshold_color_amount The color threshold (default: 16).
     * @return True if the terminal supports at least that many colors.
     */
    auto term_has_colors(
        int32_t threshold_color_amount = DEFAULT_COLOR_THRESHOLD
    ) -> bool;


    /**
     * @brief libcurl write callback to append data to a string.
     * @param contents Incoming data buffer.
     * @param size Size of each element.
     * @param nmemb Number of elements.
     * @param userp Target string buffer.
     * @return Number of bytes handled.
     */
    auto write_callback(
        void        *contents,
        size_t       size,
        size_t       nmemb,
        std::string &userp
    ) -> size_t;


    /**
     * @brief Performs a CURL request to a given URL.
     * @param curl Optional CURL handle. If nullptr, an internal handle is used.
     * @param url The target URL.
     * @param read_buffer String to store response data.
     * @return CURLcode result.
     */
    auto perform_curl(
        CURL              *curl,
        const std::string &url,
        std::string       &read_buffer
    ) -> CURLcode;


    /**
     * @brief Executes a file with arguments, optionally resolving via PATH.
     * @param file Executable name or path.
     * @param argv Argument vector.
     * @return Exit code or -1 on error.
     */
    auto execvp(
        std::string              &file,
        std::vector<std::string> &argv
    ) -> int32_t;


    /**
     * @brief Sorts a Json::Value array based on a field @p sort_by .
     * @param root The JSON array to sort.
     * @param sort_by The field to sort on.
     * @param reverse Whether to reverse the sort order.
     * @return A vector of sorted Json::Value objects.
     */
    auto sort_json(
        const Json::Value &root,
        const std::string &sort_by,
        bool               reverse = false
    ) -> std::vector<std::reference_wrapper<const Json::Value>>;


    /**
     * @brief Returns the current errno value as a human-readable string.
     * @return The error string.
     */
    auto serrno() -> std::string;


    /**
     * @brief Safe wrapper around std::getenv that returns a string.
     * @param name Environment variable name.
     * @return The variable's value or empty string.
     */
    auto get_env(const std::string &name) -> std::string;


    /**
     * @brief Expand all environment variables inside @p str .
     * @param str The string that will be expanded.
     * @return The expanded string.
     */
    auto expand_envs(const std::string &str) -> std::string;


    /**
     * @brief Resolves the full UI file path from a filename.
     * @param file_name Name of the UI file.
     * @param base_path The base path to the UI resources.
     * @return Full file path to the UI resource.
     */
    auto get_ui_file(const fs::path    &file_name,
                     const std::string &base_path) -> std::string;


    /**
     * @brief A wrapper for std::format.
     * @param fmt Format string.
     * @param args Variadic arguments.
     * @return The formatted string.
     */
    template<typename... T_Args>
    auto format(std::string_view fmt, T_Args&&... args) -> std::string
    { return std::vformat(fmt, std::make_format_args(args...)); }


    /**
     * @brief Checks if a value exists in a vector.
     * @param vec Vector to search.
     * @param obj Object to find.
     * @return True if found, false otherwise.
     */
    template<typename T>
    auto find(const std::vector<T> &vec, const T &obj) -> bool
    { return std::ranges::find(vec, obj) != vec.end(); }


    /**
     * @namespace str
     * @brief String utility functions.
     */
    namespace str {
        /**
         * @brief Splits a string into an array of 2.
         * @param str The base string.
         * @param pos The index where the string will be split.
         * @returns An array object with size 2.
         */
        auto split(std::string_view str, size_t pos) -> std::array<std::string, 2>;


        /**
         * @brief Trims whitespace from both ends of the string.
         * @param str The input string.
         * @return A new trimmed string.
         */
        auto trim(std::string_view str) -> std::string;


        /**
        * @brief Counts the occurrences of a character in a string.
        * @param str The input string to search within.
        * @param delim The character to count.
        * @return The number of times @p delim appears in @p str.
        */
        auto count(std::string_view str, char delim) -> size_t;


        /**
         * @brief Checks wheter @p str is only composed of digits.
         * @param str The input string.
         * @return Wheter @p str is only composed of digits.
         */
        auto is_digit(const std::string &str) -> bool;
    } /* namespace str */
} /* namespace utils */

#endif /* __UTILS__HH */