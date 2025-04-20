#pragma once
#ifndef UTILS_HH__
#define UTILS_HH__

#include <optional>
#include <format>
#include <string>
#include <array>
#include <gtkmm-3.0/gtkmm/enums.h>

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
} /* namespace Str */

/**
 * @namespace Utils
 * @brief A namespace containing usefull utilities
 */
namespace Utils {
    static const size_t DEFAULT_COLOR_THRESHOLD = 16;
    static const size_t DEFAULT_BUFFER_SIZE     = 256;

    struct command_out {
        std::string stdout;
        std::string stderr;
        int32_t     return_code;
        pid_t       pid;
    };

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
        size_t threshold_color_amount = DEFAULT_COLOR_THRESHOLD) -> bool;

    /**
     * @brief A write callback function for CURL
     */
    auto write_callback(
        void *contents, size_t size, size_t nmemb, std::string &userp
    ) -> size_t;

    template<typename... T_Args>
    auto
    format(std::string_view fmt, T_Args&&... args) -> std::string
    {
        return std::vformat(fmt, std::make_format_args(args...));
    }
} /* namespace Utils */

/**
 * @namespace GtkUtils
 * @brief A namespace containing utilities for the Gtkmm-3.0 library.
 */
namespace GtkUtils {
    void set_margin(Gtk::Widget &widget, std::array<int32_t, 4> margin);
    void set_margin(Gtk::Widget &widget, std::array<int32_t, 2> margin);
    void set_margin(Gtk::Widget &widget, int32_t margin);

    auto create_label_markup(const std::string &markup) -> Gtk::Label*;
    auto create_label_icon(
        const std::string &icons,
        const std::string &markup,
        Gtk::IconSize icon_size
    ) -> Gtk::Box*;
} /* namespace GtkUtils */

#endif /* utils.hh */