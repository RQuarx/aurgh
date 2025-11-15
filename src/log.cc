#include <chrono>
#include <cstring>
#include <iostream>

#include "log.hh"
#include "utils.hh"


namespace
{
    template <typename T> using tpair = std::pair<T, T>;


    auto
    string_to_loglevel(std::string_view p_str) -> LogLevel
    {
        if (p_str.contains("debug")) return LogLevel::DEBUG;
        if (p_str.contains("info")) return LogLevel::INFO;
        if (p_str.contains("warn")) return LogLevel::WARN;
        if (p_str.contains("error")) return LogLevel::ERROR;
        return LogLevel::MAX;
    }


    /* clang-format off */
     constexpr std::array<tpair<std::string_view>, 4> LABELS {{
         { "\033[1;36mdebug\033[0;0;0m", "debug" },
         { "\033[1;32minfo\033[0;0;0m ", "info " },
         { "\033[1;33mwarn\033[0;0;0m ", "warn " },
         { "\033[1;31merror\033[0;0;0m", "error" },
    }};
    /* clang-format on */
}


auto
Logger::set_log_level(std::string_view p_log_level) -> Logger &
{
    if (p_log_level.empty()) goto err;

    try
    {
        auto level { utils::to_int<std::uint32_t>(p_log_level) };

        /* `level` is not an integer */
        if (!level) throw std::exception {};

        if (static_cast<LogLevel>(*level) >= LogLevel::MAX)
        {
            log<LogLevel::WARN>("Log level too large, using default level.");
            goto err;
        }

        m_threshold_level = static_cast<LogLevel>(*level);
    }
    catch (...)
    {
        LogLevel level { string_to_loglevel(p_log_level) };

        if (level == LogLevel::MAX)
        {
            log<LogLevel::WARN>("Invalid log level {}, using default level",
                                p_log_level);
            goto err;
        }

        m_threshold_level = level;
    }

    return *this;

err:
    m_threshold_level = LogLevel::WARN;
    return *this;
}


auto
Logger::set_log_file(const std::string &p_log_file) -> Logger &
{
    if (p_log_file.empty()) return *this;

    m_log_file.open(p_log_file, std::ios_base::app);

    if (m_log_file.fail() && !m_log_file.eof())
    {
        log<LogLevel::ERROR>("Failed to open {}: {}", p_log_file,
                             std::strerror(errno));

        throw std::exception {};
    }

    return *this;
}


void
Logger::write(LogLevel           p_level,
              std::string_view   p_domain,
              const std::string &p_msg)
{
    tpair<std::string_view> labels { LABELS[static_cast<size_t>(p_level)] };

    std::string label { std::format("{} {} at \033[38;2;70;172;173m{}\033[0;0m",
                                    get_time(), labels.first, p_domain) };

    if (m_log_file.is_open())
    {
        std::string file_label { std::format("{} at {}", get_time(),
                                             labels.second, p_domain) };

        m_log_file << std::format("[{}]: {}", file_label, p_msg) << '\n';
        m_log_file.flush();
    }

    if (p_level < m_threshold_level) return;

    std::size_t label_len { label.length() };
    m_longest_label = std::max(m_longest_label, label_len);

    std::println(std::cerr, "[{}]: {}\033[1m{}\033[0m", label,
                 std::string(m_longest_label - label_len, ' '), p_msg);
}


auto
Logger::get_time() -> std::string
{
    using std::chrono::duration;
    using ms = std::chrono::milliseconds;
    using m  = std::chrono::minutes;
    using s  = std::chrono::seconds;

    duration now { std::chrono::system_clock::now().time_since_epoch() };
    ms       millis { std::chrono::duration_cast<ms>(now) % 1000 };
    m        minutes { std::chrono::duration_cast<m>(now) % 60 };
    s        seconds { std::chrono::duration_cast<s>(now) % 60 };

    return std::format("{:02}:{:02}.{:03}", minutes.count(), seconds.count(),
                       millis.count());
}


auto
GLogLevel_to_LogLevel(GLogLevelFlags p_level) -> LogLevel
{
    if ((p_level & (G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL)) != 0)
        return LogLevel::ERROR;
    if ((p_level & G_LOG_LEVEL_WARNING) != 0) return LogLevel::WARN;
    if ((p_level & (G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO)) != 0)
        return LogLevel::INFO;
    if ((p_level & G_LOG_LEVEL_DEBUG) != 0) return LogLevel::DEBUG;

    return LogLevel::INFO;
}
