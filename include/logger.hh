/**
 * @file logger.hh
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

#pragma once
#ifndef LOGGER_HPP_
#define LOGGER_HPP_

#include <unordered_map>
#include <fstream>
#include <print>

#include "utils.hh"

class ArgParser;


/**
 * @class Logger
 * @brief A class made to log to the stream with fancy colours
 *
 * The Logger class provides functionality to log messages at different severity levels.
 * It can output colored logs to the terminal if supported, or plain text logs to a file.
 * The severity levels include Debug, Info, Warn, and Error.
*/
class Logger
{
    using label_pair = std::pair<std::string_view, std::string_view>;
public:
    /**
     * @enum Level
     * @brief Enumeration representing the severity levels of logs.
     *
     * This enum is used to categorize log messages based on their severity.
    */
    enum Level : uint8_t {
        Debug,
        Info,
        Warn,
        Error,
        None
    };

    /**
     * @brief Construct the Logger class
     * @param arg_parser A pointer to an Arg::Parser instance for command-line
     * argument parsing.
     */
    explicit Logger(ArgParser &arg_parser);

    /**
     * @brief Destructor for Logger class
     *
     * Closes the opened log file
    */
    ~Logger();

    /**
     * @brief Logs a message with a specified severity level.
     * @param log_level The severity level of the log message.
     * @param fmt A format string for the log message.
     * @param args The arguments to format the log message.
     *
     * This method formats the log message using the provided format string and arguments,
     * then outputs the message to the appropriate stream (stdout or stderr) based on the log level.
     * If a log file is open, the message is also written to the log file.
    */
    template<typename... T_Args>
    void log(Level log_level, std::string_view fmt, T_Args&&... args)
    {
        std::string message = std::vformat(fmt, std::make_format_args(args...));

        m_previous_log_level = log_level;

        std::string_view label = (m_use_color
            ? m_labels.at(log_level).first
            : m_labels.at(log_level).second
        );
        if (m_log_file.is_open()) log_to_file(log_level, message);

        if (m_log_treshold == None || log_level >= m_log_treshold) {
            std::println(
                (log_level == Error ? stderr : stdout),
                "{} {} {}",
                Utils::get_current_time(), label, message
            );
            std::fflush(stdout);
        }
    }

    auto get_previous_log_level() -> Level;

private:
    static const inline std::unordered_map<Level, label_pair> m_labels = {
        { Debug, { "\e[1;37m[\e[1;36mDEBUG\e[1;37m]:\e[0;0;0m", "[DEBUG]:" } },
        { Error, { "\e[1;37m[\e[1;31mERROR\e[1;37m]:\e[0;0;0m", "[ERROR]:" } },
        { Info,  { "\e[1;37m[\e[1;32mINFO\e[1;37m]:\e[0;0;0m ", "[INFO]: " } },
        { Warn,  { "\e[1;37m[\e[1;33mWARN\e[1;37m]:\e[0;0;0m ", "[WARN]: " } },
    };
    const std::string m_log_arg = "{-l --log}";
    Level m_previous_log_level  = None;
    Level m_log_treshold        = None;

    std::ofstream m_log_file;
    bool          m_use_color;

    /**
     * @brief Logs a message to a file.
     * @param log_level The severity level of the log message.
     * @param message The message to log.
     *
     * This method writes the log message to the specified log file, prefixed with the appropriate label
     * based on the log level.
    */
    void log_to_file(Level log_level, std::string_view message);

    /**
     * @brief Checks if a level has a valid value
     * @param level The level that'll be checked.
     *
     * This function will checks if the given level is bigger than 3,
     * and if the member level variable has value,
     * if it does, it will log an error and return false.
    */
    auto is_valid_level(int32_t level) -> bool;

    /**
     * @brief Sets the member level threshold
     * @param level The level that it'll be set to
     * @returns true on success, or false on failure
    */
    auto set_leveL_threshold(int32_t level) -> bool;

    /**
     * @brief Opens the member log file with the give file path
     * @param file The file path to the log file
     * @returns true on success, or false on failure
    */
    auto open_log_file(const std::string &file) -> bool;

    /**
     * @brief Handles the parameter that is passed to -l,--log for the constructor
     * @param option The option that is passed to -l,--log
     * @returns true on success, or false on failure
    */
    auto handle_double_parameters(const std::string &option) -> bool;
};

#endif /* logger.hpp */