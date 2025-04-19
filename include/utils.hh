#pragma once
#ifndef UTILS_HH__
#define UTILS_HH__

#include <optional>
#include <string>
#include <array>


namespace Str {
    auto split(std::string_view str, size_t pos) -> std::array<std::string, 2>;
    auto is_digit(std::string_view str) -> bool;
}


namespace Utils {
    auto get_current_time() -> std::string;
    auto run_command(
        const std::string &cmd, size_t buffer_size = 256
    ) -> std::optional<std::pair<std::string, int32_t>>;
    auto term_has_colors(size_t threshold_color_amount = 16) -> bool;
    auto write_callback(void *contents, size_t size, size_t nmemb, std::string &userp) -> size_t;
}

#endif /* utils.hh */