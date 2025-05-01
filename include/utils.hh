/**
 * @file utils.hh
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
#ifndef UTILS_HH__
#define UTILS_HH__

#include <algorithm>
#include <optional>
#include <format>
#include <string>
#include <vector>
#include <array>

#include <curl/curl.h>

namespace Gtk {
    class Widget;
    class Label;
    class Box;
}

/**
 * @namespace Str
 * @brief A namespace containing str utilities
 */
namespace Str {
    /**
     * @brief Splits a string into an array of 2.
     * @param str The base string.
     * @param pos The index where the string will be split.
     * @returns An array object with size 2.
     */
     auto splitp(std::string_view str, size_t pos) -> std::array<std::string, 2>;

    /**
     * @brief Splits a string by the delim.
     * @param str The base string.
     * @param delim The char where the string will be split.
     * @returns A vector of strings.
     */
    auto splitd(const std::string &str, char delim) -> std::vector<std::string>;

    /**
     * @brief Checks if a string only consists of digits
     */
    auto is_digit(std::string_view str) -> bool;

    auto trim(std::string_view str) -> std::string;

    /**
     * @brief Counts the amount of c inside str.
     */
    auto count(std::string_view str, char c) -> size_t;
} /* namespace Str */

/**
 * @namespace Utils
 * @brief A namespace containing usefull utilities
 */
namespace Utils {
    using str_pair = std::pair<std::string, std::string>;

    static const size_t DEFAULT_COLOR_THRESHOLD = 16UZ;
    static const size_t DEFAULT_BUFFER_SIZE     = 256UZ;

    /**
     * @brief Returns the current time as "minutes:seconds:miliseconds".
     */
    auto get_current_time() -> std::string;

    /**
     * @brief Runs a command and capture its stdout, stderr, and return code.
     * @param cmd The command that will be executed.
     * @param buffer_size The buffer size used to capture the output,
     *        defaults to 256.
     * @returns An std::optional object containing the output and return code.
     */
    auto run_command(
        const std::string &cmd, size_t buffer_size = DEFAULT_BUFFER_SIZE
    ) -> std::optional<std::pair<std::string, int32_t>>;

    /**
     * @brief Checks if the terminal used by the user has colours.
     * @param threshold_color_amount The color amount used for the threshold,
     *        defaults to 16.
     */
    auto term_has_colors(
        int32_t threshold_color_amount = DEFAULT_COLOR_THRESHOLD) -> bool;

    /**
     * @brief A write callback function for CURL
     */
    auto write_callback(
        void *contents, size_t size, size_t nmemb, std::string &userp
    ) -> size_t;

    /**
     * @brief A wrapper for curl_easy_perform
     * @param curl If nullptr, the function would use its own CURL
     * @param url The url to fetch data from
     * @param read_buffer The buffer the data will be placed in
     * @returns a CURLCode
     */
    auto perform_curl(
        CURL *curl, const std::string &url, std::string &read_buffer
    ) -> CURLcode;

    /**
     * @brief Returns a quote from https://quotes-api-self.vercel.app/quote
     */
    auto get_quote(size_t max_len) -> std::string;

    /**
     * @brief Execute FILE,
     *     searching in the `PATH' environment variable if it contains no
     *     slashes, with arguments ARGV and environment from `environ'.
     */
    auto execvp(
        std::string &file, std::vector<std::string> &argv
    ) -> int32_t;

    /**
     * @brief Returns errno as an std::string.
     */
    auto serrno() -> std::string;


    template<typename... T_Args>
    auto format(std::string_view fmt, T_Args&&... args) -> std::string
    {
        return std::vformat(
            fmt, std::make_format_args(args...)
        );
    }

    template<typename T>
    auto
    find(const std::vector<T> &vec, const T &obj) -> bool
    { return std::ranges::find(vec, obj) != vec.end(); }
} /* namespace Utils */

/**
 * @namespace GtkUtils
 * @brief A namespace containing utilities for the Gtkmm-3.0 library.
 */
namespace GtkUtils {
    void set_margin(Gtk::Widget &widget, std::array<int32_t, 4> margin);
    void set_margin(Gtk::Widget &widget, int32_t margin_y, int32_t margin_x);
    void set_margin(Gtk::Widget &widget, int32_t margin);
} /* namespace GtkUtils */

#endif /* utils.hh */