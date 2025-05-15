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
#ifndef PROCESS_HH__
#define PROCESS_HH__

#include <optional>
#include <atomic>
#include <memory>
#include <string>
#include <vector>

class Logger;


class Process {
public:
    Process(
        std::string file,
        std::vector<std::string> argv,
        const std::shared_ptr<Logger> &logger,
        const std::string &cwd = ""
    );
    ~Process();

    /**
     * @brief Kills the process, or returns the return code
     *     if the process were to end before the function is called.
     * @returns -1 on failure, 0 on success / return code, or the return code.
     */
    auto kill() -> int32_t;

    /**
     * @brief Checks if the process is 'done'
     * @returns std::nullopt on failure, else the return code or -1.
     */
    [[nodiscard]]
    auto is_done() const -> std::optional<int32_t>;

private:
    std::atomic<std::shared_ptr<Logger>> ma_logger;
    std::atomic<bool>                    ma_is_running;

    pid_t       m_child_pid;
    std::string m_cwd;
};

#endif /* process.hh */