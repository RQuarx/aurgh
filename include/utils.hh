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
    auto split(std::string_view str, size_t pos) -> std::array<std::string, 2>;

    /**
     * @brief Checks if a string only consists of digits
     */
    auto is_digit(std::string_view str) -> bool;

    auto trim(std::string_view str) -> std::string;

    auto make_argv(const std::string &cmd) -> std::vector<const char*>;
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
     * @brief Returns a quote from https://quotes-api-self.vercel.app/quote
     */
    auto get_quote(size_t max_len) -> std::string;


    template<typename... T_Args>
    auto format(std::string_view fmt, T_Args&&... args) -> std::string
    {
        return std::vformat(
            fmt, std::make_format_args(args...)
        );
    }

    template<typename T>
    auto
    find(const std::vector<T> &vec, T obj) -> bool
    {
        return std::ranges::find(vec, obj) != vec.end();
        // return std::ranges::find(vec, [&obj](const T &o){
        //     return o == obj;
        // });
    }
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