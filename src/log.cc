#include <chrono>
#include <cstring>

#include "log.hh"
#include "utils.hh"


namespace
{
    auto
    string_to_loglevel(std::string_view p_str) -> LogLevel
    {
        if (p_str.contains("debug")) return DEBUG;
        if (p_str.contains("info")) return INFO;
        if (p_str.contains("warn")) return WARN;
        if (p_str.contains("error")) return ERROR;
        return MAX;
    }
}


auto
Logger::set_log_level(this Logger &self, std::string_view p_log_level)
    -> Logger &
{
    if (p_log_level.empty()) goto err;

    try
    {
        auto level { utils::to_int<uint32_t>(p_log_level) };

        /* `level` is not an integer */
        if (!level) throw std::exception {};

        if (level >= LogLevel::MAX)
        {
            self.log<WARN>("Log level too large, using default level.");
            goto err;
        }

        self.threshold_level = static_cast<LogLevel>(*level);
    }
    catch (...)
    {
        LogLevel level { string_to_loglevel(p_log_level) };

        if (level == MAX)
        {
            self.log<WARN>("Invalid log level {}, using default level",
                           p_log_level);
            goto err;
        }

        self.threshold_level = level;
    }

    return self;

err:
    self.threshold_level = LogLevel::WARN;
    return self;
}


auto
Logger::set_log_file(this Logger &self, const std::string &p_log_file)
    -> Logger &
{
    if (p_log_file.empty()) return self;

    self.log_file.open(p_log_file, std::ios_base::app);

    if (self.log_file.fail() && !self.log_file.eof())
    {
        self.log<ERROR>("Failed to open {}: {}", p_log_file,
                        std::strerror(errno));

        throw std::exception {};
    }

    return self;
}


void
Logger::write(this Logger       &self,
              LogLevel           p_level,
              std::string_view   p_domain,
              const std::string &p_msg)
{
    std::string label { std::format("{} {} at \033[38;2;70;172;173m{}\033[0;0m",
                                    get_time(), LABELS[p_level].first,
                                    p_domain) };

    if (self.log_file.is_open())
    {
        std::string file_label { std::format(
            "{} at {}", get_time(), LABELS[p_level].second, p_domain) };

        self.log_file << std::format("[{}]: {}", file_label, p_msg) << '\n';
        self.log_file.flush();
    }

    if (p_level < self.threshold_level) return;

    size_t label_len { label.length() };
    self.longest_label = std::max(self.longest_label, label_len);

    std::println(std::cerr, "[{}]: {}\033[1m{}\033[0m", label,
                 std::string(self.longest_label - label_len, ' '), p_msg);
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
        return ERROR;
    if ((p_level & G_LOG_LEVEL_WARNING) != 0) return WARN;
    if ((p_level & (G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_INFO)) != 0) return INFO;
    if ((p_level & G_LOG_LEVEL_DEBUG) != 0) return DEBUG;

    return INFO;
}
