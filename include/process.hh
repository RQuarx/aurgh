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
#include <thread>
#include <array>
#include <uv.h>

class Logger;


class Process {
public:
    Process(std::string cmd, const std::string &cwd, Logger *logger);
    ~Process();

    auto stop() -> int64_t;

    [[nodiscard]]
    auto is_running() const -> bool;

private:
    uv_loop_t*           m_loop;
    uv_process_t         m_proc{};
    uv_process_options_t m_options{};
    uv_signal_t          m_kill_signal{};
    uv_async_t           m_stop_async{};

    Logger* m_logger;

    std::atomic<bool>    m_running;
    std::atomic<int64_t> m_exit_code{0};
    std::thread          m_event_thread;

    std::string          m_cmd;
    std::string          m_cwd;
    std::array<const char*, 4> m_args = {
        "/bin/bash",
        "-c",
        nullptr,
        nullptr
    };

    void run_event_loop();

    static Process *m_instance;

    static void on_exit(uv_process_t* req, int64_t exit_status, int32_t term_signal);
    static void on_signal(uv_signal_t* handle, int32_t signum);
    static void async_stop_callback(uv_async_t* handle);
};

#endif /* process.hh */