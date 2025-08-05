#pragma once
#include <source_location>
#include <iostream>
#include <fstream>
#include <format>
#include <array>


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
    using label_pair = std::pair<std::string_view, std::string_view>;
public:
    struct StringSource
    {
        std::string_view fmt;
        std::source_location source;

        StringSource( const char          *p_fmt,
                      std::source_location p_source =
                      std::source_location::current() ) :
            fmt(p_fmt), source(p_source) {}
    };


    /**
     * p_arg_string would be a string given to the program by
     * command-line argument, the form should follow these syntax:
     *
     *  - <log_level>
     *  - <file_path>
     *  - <log_level>,<file_path>
     *
     * Example:
     *  - warn,out.log
     *  - debug
     *  - out.log
     */
    Logger( const std::string &p_arg_string );


    template<LogLevel T_Level, typename... T_Args>
    void log( StringSource p_fmt,
              T_Args  &&...p_args )
    {
        std::string msg { std::vformat(p_fmt.fmt,
                          std::make_format_args(p_args...)) };

        std::string func { p_fmt.source.function_name() };
        std::string file { p_fmt.source.file_name() };
        uint32_t line { p_fmt.source.line() };

        func = func.substr(0, func.find('('));

        if (func.contains(' '))
            func = func.substr(func.find_first_of(' ') + 1);

        std::string label {
            std::format("{} {} at \033[1m{}\033[0m( \033[1;30m{}:{}\033[0;0m )",
                         get_time(),
                         m_LABELS.at(T_Level).first,
                         func, file, line) };

        if (m_log_file.is_open()) {
            std::string file_label {
                std::format("{} at {}( {}:{} )",
                             get_time(),
                             m_LABELS.at(T_Level).second,
                             func, file, line) };

            m_log_file << std::format("[{}]: {}", file_label, msg) << std::endl;
        }

        if (T_Level < m_threshold_level) return;

        T_Level >= WARN ? std::cerr : std::cout <<
            std::format("[{}]: \033[1m{}\033[0m", label, msg) << std::endl;
    }

private:
    static constexpr std::array<label_pair, 4> m_LABELS {{
        { "\033[1;36mdebug\033[0;0;0m", "debug" },
        { "\033[1;32minfo\033[0;0;0m ", "info " },
        { "\033[1;33mwarn\033[0;0;0m ", "warn " },
        { "\033[1;31merror\033[0;0;0m", "error" },
    }};

    std::ofstream m_log_file;
    LogLevel      m_threshold_level;


    [[nodiscard]]
    auto get_time( void ) const -> const std::string;
};