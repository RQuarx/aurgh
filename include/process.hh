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

#include <atomic>
#include <string>
#include <vector>

class Logger;


class Process {
public:
    Process(
        std::string file,
        std::vector<std::string> argv,
        Logger *logger,
        const std::string &cwd = ""
    );
    // ~Process();

private:
    std::atomic<Logger*> ma_logger;
    std::atomic<bool>    ma_is_running;

    pid_t       m_child_pid;
    std::string m_cwd;
};

#endif /* process.hh */