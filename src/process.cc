/**
 * @file process.cc
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

#include "process.hh"
#include "logger.hh"

Process *Process::m_instance = nullptr;


Process::Process(std::string cmd, const std::string &cwd, Logger *logger) :
    m_loop(uv_default_loop()),
    m_logger(logger),
    m_running(false),
    m_cmd(std::move(cmd))
{
    if (cwd.empty()) {
        m_cwd = std::format("{}/.cache/aurgh/", std::getenv("HOME"));
    } else {
        m_cmd = cwd;
    }

    uv_async_init(m_loop, &m_stop_async, async_stop_callback);
    m_stop_async.data = this;

    m_args.at(2) = m_cmd.c_str();

    if (!std::filesystem::exists(m_cwd)) {
        std::filesystem::create_directory(m_cwd);
    }

    m_options.exit_cb = on_exit;
    m_options.file = m_args.at(0);
    m_options.args = const_cast<char**>(m_args.data()); /* API tweaking fr fr */
    m_options.flags = 0;
    m_options.stdio_count = 0;
    m_options.cwd = m_cwd.c_str();
    m_options.env = nullptr;

    m_instance = this;

    int32_t result = uv_spawn(m_loop, &m_proc, &m_options);
    if (result != 0) {
        m_logger->log(Logger::Error, "uv_spawn error: {}", uv_strerror(result));
        throw std::runtime_error("uv_spawn error\n");
    }

    m_running = true;
    m_logger->log(
        Logger::Debug, "Started process '{}' with PID: {}", m_cmd, m_proc.pid
    );

    uv_signal_init(m_loop, &m_kill_signal);
    uv_signal_start(&m_kill_signal, on_signal, SIGINT);
    m_kill_signal.data = this;

    m_event_thread = std::thread(&Process::run_event_loop, this);
}


Process::~Process()
{
    if (m_running) stop();

    if (m_event_thread.joinable()) {
        m_event_thread.join();
    }

    uv_close(reinterpret_cast<uv_handle_t*>(&m_stop_async), nullptr);
    uv_close(reinterpret_cast<uv_handle_t*>(&m_kill_signal), nullptr);

    uv_run(m_loop, UV_RUN_NOWAIT);
}


auto
Process::stop() -> int64_t
{
    if (m_running) {
        uv_async_send(&m_stop_async);
        return -1;
    }
    return m_exit_code.load();
}


auto
Process::is_running() const -> bool
{ return m_running; }


void
Process::on_exit(uv_process_t* req, int64_t exit_status, int32_t  /*term_signal*/)
{
    auto *self = m_instance;

    self->m_running = false;
    self->m_exit_code.store(exit_status);
    uv_close(reinterpret_cast<uv_handle_t*>(req), nullptr);
    uv_signal_stop(&self->m_kill_signal);
    uv_stop(self->m_loop);
}


void
Process::on_signal(uv_signal_t* handle, int32_t  /*signum*/)
{
    auto *self = static_cast<Process*>(handle->data);
    self->stop();
}


void
Process::async_stop_callback(uv_async_t *handle)
{
    auto *self = static_cast<Process*>(handle->data);
    if (self->is_running()) {
        uv_process_kill(&self->m_proc, SIGINT);
    }
}


void
Process::run_event_loop()
{ uv_run(m_loop, UV_RUN_DEFAULT); }