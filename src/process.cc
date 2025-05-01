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

#include <algorithm>
#include <filesystem>
#include <cstring>
#include <format>

#include <unistd.h>

#include "process.hh"
#include "logger.hh"

static const std::string DEFAULT_CWD =
    std::format("{}/.cache/aurgh/", std::getenv("HOME"));


Process::Process(
    std::string file,
    std::vector<std::string> argv,
    Logger *logger,
    const std::string &cwd
) :
    ma_logger(logger),
    m_cwd(cwd.empty() ? DEFAULT_CWD : cwd)
{
    pid_t pid = fork();

    if (pid == -1) {
        ma_logger.load()->log(
            Logger::Error, "Failed to create child process: {}", strerror(errno)
        );
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { /* Child process */
        if (!std::filesystem::exists(m_cwd)) {
            std::filesystem::create_directory(m_cwd);
        }

        if (chdir(m_cwd.c_str()) != 0) {
            ma_logger.load()->log(
                Logger::Error, "Failed to change directory: {}", strerror(errno)
            );
            exit(EXIT_FAILURE);
        }

        Utils::execvp(file, argv);

        ma_logger.load()->log(
            Logger::Error,
            "'execvp' failed to run: {}",
            strerror(errno)
        );
        exit(EXIT_FAILURE);
    } else { /* Parent */
        m_child_pid = pid;
    }
}