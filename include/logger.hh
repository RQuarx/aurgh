#pragma once
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <queue>
#include <source_location>
#include <thread>

#include <glib.h>

#include "utils.hh"


enum Level : std::uint8_t
{
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};


class LoggerPrinter;
class Logger
{
    friend class LoggerPrinter;

    struct LogObject
    {
        std::chrono::time_point<std::chrono::system_clock> time;
        Level                                              level;
        std::string                                        domain;
        std::source_location                               source;
        std::string                                        message;
    };

public:
    Logger(std::string_view             threshold_level,
           const std::filesystem::path &log_file);
    ~Logger();


    [[nodiscard]]
    auto operator[](Level                level,
                    std::string          domain,
                    std::source_location source
                    = std::source_location::current()) noexcept
        -> LoggerPrinter;


    [[nodiscard]]
    static auto GLogLevel_to_Level(GLogLevelFlags level) noexcept -> Level;


private:
    Level         m_threshold;
    std::ofstream m_log_file;

    std::queue<LogObject> m_log_queue;
    std::mutex            m_queue_mtx;

    std::condition_variable m_cv;
    std::jthread            m_worker;


    void process_queue(std::stop_token stop_token);


    template <typename... T_Args>
    void
    push_log(LogObject &&object)
    {
        {
            std::scoped_lock lock { m_queue_mtx };
            m_log_queue.push(std::move(object));
        }
        m_cv.notify_one();
    }
};


class LoggerPrinter
{
    friend class Logger;

public:
    LoggerPrinter(const LoggerPrinter &)                     = delete;
    auto operator=(const LoggerPrinter &) -> LoggerPrinter & = delete;

    LoggerPrinter(LoggerPrinter &&)                     = default;
    auto operator=(LoggerPrinter &&) -> LoggerPrinter & = default;


    template <typename... T_Args>
    void
    operator()(std::format_string<T_Args...> fmt, T_Args &&...args)
    {
        if (m_parent == nullptr) return;

        m_obj.message = std::format(fmt, std::forward<T_Args>(args)...);
        m_parent->push_log(std::move(m_obj));
    }

private:
    Logger           *m_parent;
    Logger::LogObject m_obj;


    LoggerPrinter(Logger              *parent,
                  Level                level,
                  std::string          domain,
                  std::source_location source);
};


inline Logger logger { utils::get_env("LOG_LEVEL"),
                       utils::get_env("LOG_FILE") };
