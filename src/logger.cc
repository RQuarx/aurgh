#include <filesystem>
#include <algorithm>
#include <thread>

#include "arg_parser.hh"
#include "logger.hh"


Logger::Logger(ArgParser &arg_parser) :
    m_use_color(Utils::term_has_colors())
{
    std::string option;
    if (arg_parser.option_arg(option, { "-l", "--log" })) {
        if (option.contains(',')) {
            /* Exits with error code 1 if it fails */
            if (!handle_double_parameters(option)) {
                exit(EXIT_FAILURE);
            }
        } else {
            if (Str::is_digit(option)) {
                if (!set_leveL_threshold(std::stoi(option))) {
                    exit(EXIT_FAILURE);
                }
            } else if (!open_log_file(option)) {
                exit(EXIT_FAILURE);
            }
        }
    }

    if (m_log_treshold == Level::None) m_log_treshold = Level::Warn;
    log(
        Level::Debug,
        "Logger instace successfully created with a log level of {}.",
        static_cast<int32_t>(m_log_treshold)
    );
}


Logger::~Logger()
{ if (m_log_file.is_open()) m_log_file.close(); }


auto
Logger::is_valid_level(int32_t level) -> bool
{
    if (level > 3) {
        log(Level::Error, "Invalid log level passed to {}!", m_log_arg);
        return false;
    }

    if (m_log_treshold != Level::None) {
        log(Level::Error, "Duplicate parameters passed to {}!", m_log_arg);
        return false;
    }
    return true;
}


auto
Logger::set_leveL_threshold(int32_t level) -> bool
{
    if (!is_valid_level(level)) return false;
    m_log_treshold = static_cast<Level>(level);
    return true;
}


auto
Logger::open_log_file(const std::string &file) -> bool
{
    if (m_log_file.is_open()) {
        log(Level::Error, "Duplicate options to {}!", m_log_arg);
        return false;
    }

    try {
        if (std::filesystem::exists(file)) {
            m_log_file.open(file, std::ios_base::app);
        } else {
            m_log_file.open(file);
        }
        return true;
    } catch (const std::exception &e) {
        log(Level::Error, "Failed to open log file {}: {}", file, e.what());
        return false;
    }
}


auto
Logger::handle_double_parameters(const std::string &option) -> bool
{
    return std::ranges::all_of(Str::split(option, option.find(',')),
        [this](const std::string &param){
            if (Str::is_digit(param)) {
                return set_leveL_threshold(std::stoi(param));
            }

            return open_log_file(param);
        }
    );
}


void
Logger::log_to_file(Level log_level, std::string_view message)
{
    /* Log to file with jthread / multithreading thing */
    std::jthread([this, log_level, message]() {
        std::string_view label = m_labels.at(log_level).second;
        std::println(
            m_log_file, "{} {} {}", Utils::get_current_time(), label, message
        );
    });
}


auto
Logger::get_previous_log_level() -> Level
{ return m_previous_log_level; }