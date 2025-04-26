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

#include <sys/wait.h>
#include "process.hh"
#include "logger.hh"


Process::Process(const std::string &cmd, Logger *logger) :
    m_logger(logger),
    m_pid(fork()),
    m_cmd(cmd)
{
    if (m_pid < 0) {
        m_logger->log(Logger::Error, "Failed to fork process: {}", strerror(errno));
    }

    if (m_pid == 0) {
        m_logger->log(Logger::Debug, "Starting process '{}'", cmd);
        auto command = Str::split(cmd, cmd.find_first_of(' '));

        execlp(command.at(0).c_str(), cmd.c_str());
        m_logger->log(Logger::Error, "execlp() failed: {}", strerror(errno));
        exit(EXIT_FAILURE);
    } else {
        int32_t status;

        if (waitpid(m_pid, &status, 0) == -1) {
            m_logger->log(
                Logger::Error, "waitpid() failed: {}", strerror(errno)
            );
        }
    }


}