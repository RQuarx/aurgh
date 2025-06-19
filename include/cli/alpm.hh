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
#ifndef __ALPM__HH
#define __ALPM__HH

#include <alpm.h>
#include "types.hh"

class Logger;


class Alpm
{
public:
    Alpm(
        const str                &root_path,
        const str                &db_path,
        str                       prefix,
        alpm_errno_t             &err
    );

    ~Alpm();


    /**
     * @brief Removes packages @p pkgs from the local database.
     * @param pkgs The packages that will be removed.
     * @return true on success, or false on failure.
     */
    [[nodiscard]]
    auto remove_packages(const vec<str> &pkgs) -> bool;


    /**
     * @brief Downloads and install package @p pkg to the system.
     * @param pkg The package thatll be downloaded and installed.
     * @return true on success, or false on failure.
     */
    [[nodiscard]]
    auto download_and_install_package(const str &pkg) -> bool;


    /**
     * @brief Returns member m_str_error.
     * @return A const std::string object.
     */
    [[nodiscard]]
    auto get_str_err() const -> str;


    void set_removal_flags(i32 flag);

private:
    str
        m_str_err,
        m_prefix;

    alpm_errno_t         m_err;
    alpm_handle_t       *m_handle{};
    i32                  m_removal_flags;
};

#endif /* __ALPM__HH */