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
#ifndef ALPM_HH__
#define ALPM_HH__

#include <string>
#include <vector>

#include <alpm.h>


class Alpm
{
public:
    Alpm(
        const std::string &root_path,
        const std::string &db_path,
        alpm_errno_t      &err
    );

    ~Alpm();


    [[nodiscard]]
    auto remove_packages(const std::vector<std::string> &pkgs) -> bool;


    [[nodiscard]]
    auto install_packages(const std::vector<std::string> &paths) -> bool;


    void set_removal_flags(int32_t flag);

private:

    alpm_errno_t   m_err;
    alpm_handle_t *m_handle{};
    int32_t        m_removal_flags;
};

#endif /* alpm.hh */