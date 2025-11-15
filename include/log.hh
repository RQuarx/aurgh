#pragma once
#include <array>
#include <format>
#include <fstream>
#include <source_location>

#include <glib.h>


enum LogLevel : unsigned char
{
    DEBUG = 0,
    INFO  = 1,
    WARN  = 2,
    ERROR = 3,
    MAX   = 4
};


[[nodiscard]]
auto GLogLevel_to_LogLevel(GLogLevelFlags p_level) -> LogLevel;


class Logger
{
    template <typename T> using tpair = std::pair<T, T>;

public:
    template <typename... T_Args> struct StringSource
    {
        std::format_string<T_Args...> fmt;
        std::string_view              func;

        constexpr StringSource(const char                 *p_fmt,
                               const std::source_location &p_source
                               = std::source_location::current())
            : fmt(p_fmt), func(p_source.function_name())
        {
        }
    };

    template <typename... T_Args>
    using StringSource_t = std::type_identity_t<StringSource<T_Args...>>;


    auto set_log_level(this Logger &self, std::string_view p_log_level)
        -> Logger &;


    auto set_log_file(this Logger &self, const std::string &p_log_file)
        -> Logger &;


    template <LogLevel T_Level, typename... T_Args>
    void
    log(this Logger &self, StringSource_t<T_Args...> p_fmt, T_Args &&...p_args)
    {
        std::string_view func { p_fmt.func.substr(0, p_fmt.func.find('(')) };

        for (char delim : " > ")
            if (auto pos { func.find(delim) }; pos != std::string_view::npos)
                func = func.substr(pos + 1);

        self.write(T_Level, func,
                   std::format(p_fmt.fmt, std::forward<T_Args>(p_args)...));
    }


    template <LogLevel T_Level>
    void
    log(this Logger                &self,
        std::string_view            p_string,
        const std::source_location &p_source = std::source_location::current())
    {
        std::string_view func { p_source.function_name() };

        func = func.substr(0, func.find('('));

        for (char delim : " > ")
            if (auto pos { func.find(delim) }; pos != std::string_view::npos)
                func = func.substr(pos + 1);

        self.write(T_Level, func, std::format("{}", p_string));
    }


    template <typename... T_Args>
    void
    glog(this Logger                  &self,
         LogLevel                      p_level,
         std::string_view              p_domain,
         std::format_string<T_Args...> p_fmt,
         T_Args &&...p_args)
    {
        self.write(p_level, p_domain,
                   std::format(p_fmt, std::forward<T_Args>(p_args)...));
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
    std::size_t   longest_label { 0 };


    void write(this Logger       &self,
               LogLevel           p_level,
               std::string_view   p_domain,
               const std::string &p_msg);


    [[nodiscard]]
    static auto get_time() -> std::string;
};


inline Logger logger;
