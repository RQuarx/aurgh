/**
 * @file process.hh
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
#ifndef PROCESS_HH__
#define PROCESS_HH__

#include <string>

class Logger;


class Process
{
public:
    explicit Process(const std::string &cmd, Logger *logger);

    /**
     * @brief Stops the process from running.
     * @returns -1 if the process is stopped,
     *      or a return code if the process already ends.
     */
    auto stop() -> int32_t;

private:
    Logger          *m_logger;
    pid_t            m_pid;
    std::string_view m_cmd;
};

#endif /* process.hh */