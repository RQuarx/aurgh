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

#pragma once
#ifndef LOG_HH__
#define LOG_HH__

#include <fstream>
#include <string>

#include <json/reader.h>
#include <json/writer.h>
#include <json/value.h>


/**
 * @class Log
 * @brief Used to log to a file.
 */
class Log
{
public:
    explicit Log(const std::string &log_file_path);

    template<typename... T_Args>
    void write(bool is_error, std::string_view fmt, T_Args&&... args)
    {
        Json::Value log_arr { Json::arrayValue };

        if (m_log_file_i.peek() == std::ifstream::traits_type::eof()) {
            m_log_file_o << Json::arrayValue;
        }
        m_log_file_i >> log_arr;


        std::string msg = std::vformat(fmt, std::make_format_args(args...));
        Json::Value json;

        json["type"]    = (is_error ? "error" : "info");
        json["message"] = msg;

        log_arr.append(json);

        m_log_file_o << log_arr;
    }

private:
    std::ofstream m_log_file_o;
    std::ifstream m_log_file_i;

};

#endif /* log.hh */