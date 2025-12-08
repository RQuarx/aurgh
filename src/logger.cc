#include <iostream>
#include <thread>
#include <utility>

#include "logger.hh"
#include "utils.hh"


namespace
{
    template <typename T> using tpair = std::pair<T, T>;


    auto
    string_is_equals(std::string_view   a,
                     std::string_view   b,
                     const std::locale &loc = std::locale {}) -> bool
    {
        if (a.size() != b.size()) return false;

        const auto &facet { std::use_facet<std::ctype<char>>(loc) };

        return std::ranges::equal(
            a, b, [&facet](char c1, char c2) -> bool
            { return facet.tolower(c1) == facet.tolower(c2); });
    }


    auto
    string_to_loglevel(std::string_view string) -> std::uint8_t
    {
        if (string_is_equals(string, "trace")) return Level::TRACE;
        if (string_is_equals(string, "debug")) return Level::DEBUG;
        if (string_is_equals(string, "info")) return Level::INFO;
        if (string_is_equals(string, "warn")) return Level::WARN;
        if (string_is_equals(string, "error")) return Level::FATAL;
        if (string_is_equals(string, "fatal")) return Level::ERROR;
        return std::numeric_limits<std::uint8_t>::max();
    }


    auto
    get_time(const std::chrono::time_point<std::chrono::system_clock> &now)
        -> std::string
    {
        using std::chrono::duration;
        using ms = std::chrono::milliseconds;
        using m  = std::chrono::minutes;
        using s  = std::chrono::seconds;

        auto dura { now.time_since_epoch() };
        ms   millis { std::chrono::duration_cast<ms>(dura) % 1000 };
        m    minutes { std::chrono::duration_cast<m>(dura) % 60 };
        s    seconds { std::chrono::duration_cast<s>(dura) % 60 };

        return std::format("{:02}:{:02}.{:03}", minutes.count(),
                           seconds.count(), millis.count());
    }


    /* clang-format off */
    constexpr std::array<tpair<std::string_view>, 6> LABELS {{
         { "\033[38;2;156;163;175mtrace\033[0m", "trace" },
         { "\033[38;2;59;130;246mdebug\033[0m",  "debug" },
         { "\033[38;2;34;211;238minfo\033[0m ",  "info " },
         { "\033[38;2;250;204;21mwarn\033[0m ",  "warn " },
         { "\033[38;2;239;68;68merror\033[0m",   "error" },
         { "\033[38;2;192;38;211mfatal\033[0m",  "fatal" }
    }};
    /* clang-format on */
}


Logger::Logger(std::string_view             threshold_level,
               const std::filesystem::path &log_file)
    : m_threshold { Level::WARN }, m_log_file { log_file, std::ios::app } 
{
    if (!threshold_level.empty()) try
        {
            auto level { utils::to_int<std::uint32_t>(threshold_level) };

            /* `level` is not an integer */
            if (!level) throw std::exception {};

            if (static_cast<Level>(*level) >= Level::FATAL)
            {
                (*this)[Level::WARN, "logger"](
                    "Threshold level too large, using default level");

                goto end;
            }

            m_threshold = static_cast<Level>(*level);
        }
        catch (...)
        {
            Level level { string_to_loglevel(threshold_level) };

            if (level == std::numeric_limits<std::uint8_t>::max())
            {
                (*this)[Level::WARN, "logger"](
                    "Invalid threshold level {}, using default level",
                    threshold_level);

                goto end;
            }

            m_threshold = level;
        }

end:

    m_worker = std::jthread([this](std::stop_token st) -> void
                            { process_queue(std::move(st)); });
}


Logger::~Logger()
{
    m_worker.request_stop();
    m_cv.notify_one();
}


auto
Logger::operator[](Level                level,
                   std::string          domain,
                   std::source_location source) noexcept -> LoggerPrinter
{
    return LoggerPrinter { this, level, std::move(domain), source };
}


void
Logger::process_queue(std::stop_token stop_token)
{
    while (!stop_token.stop_requested() || !m_log_queue.empty())
    {
        std::unique_lock lock { m_queue_mtx };

        m_cv.wait(
            lock, [&] -> bool
            { return stop_token.stop_requested() || !m_log_queue.empty(); });

        while (!m_log_queue.empty())
        {
            const auto obj { std::move(m_log_queue.front()) };
            m_log_queue.pop();

            lock.unlock();

            const std::string time_str { get_time(obj.time) };

            const auto  level_index { static_cast<std::size_t>(obj.level) };
            const auto &label_colored { LABELS[level_index].first };
            const auto &label_raw { LABELS[level_index].second };

            const bool is_error_or_fatal { obj.level == Level::ERROR
                                           || obj.level == Level::FATAL };

            std::string formatted_console;
            std::string formatted_file;

            if (is_error_or_fatal)
            {
                formatted_console = std::format(
                    "{} [{} at \033[38;2;70;172;173m{}:{}\033[0m]: "
                    "\033[1m{}\033[0m",
                    time_str, label_colored, obj.source.file_name(),
                    obj.source.line(), obj.message);

                formatted_file = std::format("{} [{} at {}:{}]: {}", time_str,
                                             label_raw, obj.source.file_name(),
                                             obj.source.line(), obj.message);
            }
            else
            {
                formatted_console = std::format(
                    "{} [{} at \033[38;2;70;172;173m{}\033[0m]: "
                    "\033[1m{}\033[0m",
                    time_str, label_colored, obj.domain, obj.message);

                formatted_file
                    = std::format("{} [{} at {}]: {}", time_str, label_raw,
                                  obj.domain, obj.message);
            }

            std::clog << formatted_console << '\n';

            if (m_log_file.is_open())
            {
                m_log_file << formatted_file << '\n';
                m_log_file.flush();
            }

            lock.lock();
        }
    }
}


auto
Logger::GLogLevel_to_Level(GLogLevelFlags level) noexcept -> Level
{
    switch (level)
    {
    case G_LOG_LEVEL_CRITICAL: [[fallthrough]];
    case G_LOG_FLAG_FATAL:     return Level::FATAL;
    case G_LOG_LEVEL_ERROR:    return Level::ERROR;
    case G_LOG_LEVEL_WARNING:  return Level::WARN;
    case G_LOG_LEVEL_MESSAGE:  return Level::TRACE;
    case G_LOG_LEVEL_DEBUG:    return Level::DEBUG;
    case G_LOG_LEVEL_MASK:     [[fallthrough]];
    case G_LOG_FLAG_RECURSION: [[fallthrough]];
    case G_LOG_LEVEL_INFO:     return Level::INFO;
    }
}


LoggerPrinter::LoggerPrinter(Logger              *parent,
                             Level                level,
                             std::string          domain,
                             std::source_location source)
    : m_parent { parent }, m_obj { .time    = std::chrono::system_clock::now(),
                                   .level   = level,
                                   .domain  = std::move(domain),
                                   .source  = source,
                                   .message = "" }
{
}
