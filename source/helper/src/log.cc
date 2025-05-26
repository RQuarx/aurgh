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

#include "log.hh"


Log::Log(const std::string &log_file_path)
{
    namespace fs = std::filesystem;

    if (fs::exists(log_file_path)) {
        fs::remove(log_file_path);
    }

    if (!fs::exists(log_file_path)) {
        fs::path log_file = log_file_path;

        if (!log_file.parent_path().string().empty()) {
            fs::create_directory(log_file.parent_path());
        }
    }

    try {
        m_log_file_o.open(log_file_path);
        m_log_file_i.open(log_file_path);
    } catch (const std::exception &e) {
        throw std::runtime_error(e.what());
    }
}