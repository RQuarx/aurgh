#pragma once
#include <array>
#include <format>
#include <fstream>
#include <iostream>
#include <source_location>


enum LogLevel : unsigned char
{
    DEBUG = 0,
    INFO  = 1,
    WARN  = 2,
    ERROR = 3,
    MAX   = 4
};


class Logger
{
    template <typename T> using tpair = std::pair<T, T>;

public:
    struct StringSource
    {
        std::string_view     fmt;
        std::source_location source;

        consteval StringSource(const char          *p_fmt,
                               std::source_location p_source
                               = std::source_location::current())
            : fmt(p_fmt), source(p_source)
        {
        }
    };


    auto set_log_level(this Logger &self, std::string_view p_log_level)
        -> Logger &;


    auto set_log_file(this Logger &self, const std::string &p_log_file)
        -> Logger &;


    template <LogLevel T_Level, typename... T_Args>
    void
    log(this Logger &self, StringSource p_fmt, T_Args &&...p_args)
    {
        std::string msg { std::vformat(p_fmt.fmt,
                                       std::make_format_args(p_args...)) };

        std::string_view func { p_fmt.source.function_name() };
        std::string_view file { p_fmt.source.file_name() };
        uint_least32_t   line { p_fmt.source.line() };

        func = func.substr(0, func.find('('));

        if (auto pos { func.find('>') }; pos != std::string_view::npos)
            func = func.substr(pos + 1);

        if (auto pos { func.find(' ') }; pos != std::string_view::npos)
            func = func.substr(pos + 1);

        std::string label { std::format(
            "{} {} at \033[1m{}\033[0m( \033[1;30m{}:{}\033[0;0m )", get_time(),
            LABELS[T_Level].first, func, file, line) };

        if (self.log_file.is_open())
        {
            std::string file_label { std::format(
                "{} at {}( {}:{} )", get_time(), LABELS[T_Level].second, func,
                file, line) };

            self.log_file << std::format("[{}]: {}", file_label, msg) << '\n';
            self.log_file.flush();
        }

        if (T_Level < self.threshold_level) return;
        std::println(std::cerr, "[{}]: \033[1m{}\033[0m", label, msg);
    }

private:
    /* clang-format off */
    static constexpr std::array<tpair<std::string_view>, 4> LABELS {{
         { "\033[1;36mdebug\033[0;0;0m", "debug" },
         { "\033[1;32minfo\033[0;0;0m ", "info " },
         { "\033[1;33mwarn\033[0;0;0m ", "warn " },
         { "\033[1;31merror\033[0;0;0m", "error" },
    }}; /* clang-format on */

    std::ofstream log_file;
    LogLevel      threshold_level { WARN };


    [[nodiscard]]
    static auto get_time() -> std::string;
};


inline Logger logger;
