#pragma once
#include <format>
#include <fstream>
#include <source_location>

#include <glib.h>


enum class LogLevel : unsigned char
{
    DEBUG = 0,
    INFO  = 1,
    WARN  = 2,
    ERROR = 3,
    MAX   = 4
};


/** Convert a GLib `GLogLevelFlags` value to the `LogLevel`. */
[[nodiscard]]
auto GLogLevel_to_LogLevel(GLogLevelFlags p_level) -> LogLevel;


class Logger
{
public:
    /**
     * Helper struct to store a format string and the source function name.
     *
     * [note]--------------------------------------------------------------
     *
     * Used internally for type-safe logging macros.
     */
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


    /**
     * Set the logger threshold level.
     *
     * [note]-------------------------
     *
     * Messages below the passed level are not printed to console.
     *
     * [params]-----------------------
     *
     * `p_log_level`:
     *   Either numeric or string log level ("debug", "info", etc.)
     */
    auto set_log_level(std::string_view p_log_level) -> Logger &;


    /**
     * Set the log file path.
     *
     * [note]----------------
     *
     * Throws std::exception if the file cannot be opened.
     *
     * [params]--------------
     *
     * `p_log_file`:
     *   Path to the log file.
     */
    auto set_log_file(const std::string &p_log_file) -> Logger &;


    template <LogLevel T_Level, typename... T_Args>
    void
    log(StringSource_t<T_Args...> p_fmt, T_Args &&...p_args)
    {
        std::string_view func { p_fmt.func.substr(0, p_fmt.func.find('(')) };

        for (char delim : " > ")
            if (auto pos { func.find(delim) }; pos != std::string_view::npos)
                func = func.substr(pos + 1);

        write(T_Level, func,
              std::format(p_fmt.fmt, std::forward<T_Args>(p_args)...));
    }


    template <typename... T_Args>
    void
    glog(LogLevel                      p_level,
         std::string_view              p_domain,
         std::format_string<T_Args...> p_fmt,
         T_Args &&...p_args)
    {
        write(p_level, p_domain,
              std::format(p_fmt, std::forward<T_Args>(p_args)...));
    }

private:
    std::ofstream m_log_file;
    LogLevel      m_threshold_level { LogLevel::WARN };
    std::size_t   m_longest_label { 0 };


    void write(LogLevel           p_level,
               std::string_view   p_domain,
               const std::string &p_msg);


    [[nodiscard]]
    static auto get_time() -> std::string;
};


inline Logger logger;
