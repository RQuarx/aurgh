#include <cstring>
#include <chrono>
#include "log.hh"


namespace
{
    using str_pair = std::pair<std::string, std::string>;


    auto
    string_to_loglevel( const std::string &p_str ) -> LogLevel
    {
        if (p_str.contains("debug")) return DEBUG;
        if (p_str.contains("info" )) return INFO;
        if (p_str.contains("warn" )) return WARN;
        if (p_str.contains("error")) return ERROR;
        return MAX;
    }


    auto
    split_string( const std::string &str, const size_t &idx ) -> str_pair
    {
        if (idx == 0 || idx == std::string::npos)
            return { str, "" };
        auto a { str.substr(0,  idx) };
        auto b { str.substr(idx + 1) };
        return { a, b };
    }
}


Logger::Logger( const std::string &p_log_level, const std::string &p_log_file )
{
    /* The input to the ctor should be either 'n', 'm', 'n,m', or 'm,n',
       where n is a number in a range of 0 -> 3, or a log-level,
       and m is a file path for the log file.
    */

    if (p_log_level.empty()) {
        m_threshold_level = WARN;
    } else {
        try {
            int32_t level { std::stoi(p_log_level) };
            if (level >= MAX) {
                log<WARN>("Log level too large, using default 'warn' level.");
                throw std::exception(); /* Trigger the catch */
            } else {
                m_threshold_level = static_cast<LogLevel>(level);
            }
        } catch (...) {
            LogLevel level { string_to_loglevel(p_log_level) };

            if (level == MAX) {
                log<WARN>("Invalid log level {}, using default 'warn' level",
                           p_log_level);
                m_threshold_level = WARN;
            } else m_threshold_level = level;
        }
    }

    if (!p_log_file.empty()) {
        m_log_file.open(p_log_file, std::ios_base::app);
        if (m_log_file.fail() && !m_log_file.eof()) {
            log<ERROR>("Failed to open {}: {}",
                        p_log_file, std::strerror(errno));
            exit(1);
        }
    }

    log<DEBUG>("Logger instance successfully created with a log-level of {}",
                m_LABELS.at(m_threshold_level).second);
}


auto
Logger::get_time( void ) const -> const std::string
{
    using std::chrono::duration;
    using ms = std::chrono::milliseconds;
    using m  = std::chrono::minutes;
    using s  = std::chrono::seconds;

    duration now { std::chrono::system_clock::now().time_since_epoch() };
    ms millis    { std::chrono::duration_cast<ms>(now) % 1000 };
    m  minutes   { std::chrono::duration_cast<m >(now) % 60 };
    s  seconds   { std::chrono::duration_cast<s >(now) % 60 };

    return std::format("{:02}:{:02}.{:03}",
                        minutes.count(), seconds.count(), millis.count());
}