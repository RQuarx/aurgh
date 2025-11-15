#pragma once
#include <expected>
#include <format>
#include <string>

#include <curl/curl.h>


namespace Json { class Value; }


namespace utils
{
    template <typename T_Err>
    auto
    unexpected(T_Err err) -> std::unexpected<T_Err>
    {
        return std::unexpected<T_Err>(err);
    }


    template <typename... T_Args>
    auto
    unexpected(std::string_view p_fmt, T_Args &&...p_args)
        -> std::unexpected<std::string>
    {
        std::string msg { std::vformat(p_fmt,
                                       std::make_format_args(p_args...)) };
        return std::unexpected<std::string>(msg);
    }


    auto get_env(const std::string &p_key) -> std::string;


    template <typename T_IntType>
    [[nodiscard]]
    auto
    to_int(std::string_view p_string) -> std::optional<T_IntType>
    {
        T_IntType val;
        if (std::from_chars(p_string.begin(), p_string.end(), val).ec
            == std::errc {})
            return val;

        return std::nullopt;
    }


    auto get_prefix_path() -> std::string;


    auto perform_curl(const char *p_url)
        -> std::expected<std::string, CURLcode>;
}


namespace Json { auto from_string(const std::string &p_str) -> Json::Value; }
