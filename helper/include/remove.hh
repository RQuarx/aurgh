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
#include <string>
#include <vector>
#ifndef REMOVE_HH__
#define REMOVE_HH__

#include <alpm.h>


auto remove(
    alpm_handle_t                  *handle,
    const alpm_errno_t             *alpm_errno,
    const std::vector<std::string> &pkgs
) -> bool;

void remove_pkg(
    alpm_handle_t                  *handle,
    alpm_db_t                      *local_db,
    const alpm_errno_t             *alpm_errno,
    const std::vector<std::string> &pkgs
);

#endif /* remove.hh */