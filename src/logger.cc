/**
 * aurgh Copyright (C) 2025 RQuarx
 *
 * This file is part of aurgh
 *
 * aurgh is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * aurgh is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with aurgh. If not, see <https://www.gnu.org/licenses/>.
 */

#include <filesystem>
#include <algorithm>
#include <utility>
#include <thread>

#include "logger.hh"
#include "utils.hh"
#include "data.hh"


Logger::Logger() :
    m_use_color(PRETTY_LOGGING)
{
    str log_option = data::arg_parser->get_option("log");
    if (!log_option.empty()) [[likely]] {
        if (log_option.contains(',')) [[unlikely]] {
            /* Exits with error code 1 if it fails */
            if (!handle_double_parameters(log_option)) [[unlikely]] {
                exit(EXIT_FAILURE);
            }
        } else {
            if (utils::str::is_digit(log_option)) [[likely]] {
                if (!set_leveL_threshold(std::stoi(log_option))) [[unlikely]] {
                    exit(EXIT_FAILURE);
                }
            } else if (!open_log_file(log_option)) [[unlikely]] {
                exit(EXIT_FAILURE);
            }
        }
    }

    if (m_log_treshold == Level::None) [[likely]] m_log_treshold = Level::Warn;
    log(
        Level::Debug,
        "Logger instace successfully created with a log level of {}.",
        std::to_underlying(m_log_treshold)
    );
}


Logger::~Logger()
{ if (m_log_file.is_open()) m_log_file.close(); }


auto
Logger::is_valid_level(i32 level) -> bool
{
    if (level > 3) [[unlikely]] {
        log(Level::Error, "Invalid log level passed to {}!", LOG_ARG);
        return false;
    }

    if (m_log_treshold != Level::None) [[unlikely]] {
        log(Level::Error, "Duplicate parameters passed to {}!", LOG_ARG);
        return false;
    }
    return true;
}


auto
Logger::set_leveL_threshold(i32 level) -> bool
{
    if (!is_valid_level(level)) [[unlikely]] return false;
    m_log_treshold = Level(level);
    return true;
}


auto
Logger::open_log_file(const str &file) -> bool
{
    if (m_log_file.is_open()) [[unlikely]] {
        log(Level::Error, "Duplicate options to {}!", LOG_ARG);
        return false;
    }

    try {
        if (std::filesystem::exists(file)) [[likely]] {
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
Logger::handle_double_parameters(const str &option) -> bool
{
    return std::ranges::all_of(utils::str::split(option, option.find(',')),
        [this](const str &param){
            if (utils::str::is_digit(param)) [[likely]] {
                return set_leveL_threshold(std::stoi(param));
            }

            return open_log_file(param);
        }
    );
}


void
Logger::log_to_file(Level log_level, str_view message)
{
    std::jthread([this, log_level, message]() {
        str_view label = LABELS.at(log_level).second;
        std::println(
            m_log_file, "{} {} {}", get_current_time(), label, message
        );
    });
}


auto
Logger::get_previous_log_level() -> Level
{ return m_previous_log_level; }


auto
Logger::get_current_time() -> str
{
    using std::chrono::duration;
    using ms = std::chrono::milliseconds;
    using m  = std::chrono::minutes;
    using s  = std::chrono::seconds;

    duration now = std::chrono::system_clock::now().time_since_epoch();
    ms millis    = std::chrono::duration_cast<ms>(now) % 1000;
    m minutes    = std::chrono::duration_cast<m>(now) % 60;
    s seconds    = std::chrono::duration_cast<s>(now) % 60;

    return std::format(
        "{:02}:{:02}.{:03}",
        minutes.count(), seconds.count(), millis.count()
    );
}