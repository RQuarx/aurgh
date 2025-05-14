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
#include <print>

#include <alpm.h>

#include "remove.hh"


auto
remove(
    alpm_handle_t                  *handle,
    const alpm_errno_t             *alpm_errno,
    const std::vector<std::string> &pkgs
) -> bool
{
    int32_t      flags = ALPM_TRANS_FLAG_CASCADE | ALPM_TRANS_FLAG_RECURSE;
    alpm_list_t *data{};

    if (alpm_trans_init(handle, flags) < 0) {
        std::print(
        R"({{"type": "error", "reason": "Failed to init transaction {}"}},)",
            alpm_strerror(*alpm_errno)
        );
        return false;
    }

    alpm_db_t *local_db = alpm_get_localdb(handle);

    remove_pkg(handle, local_db, alpm_errno, pkgs);

    if (alpm_trans_prepare(handle, &data) < 0) {
        std::print(
    R"({{"type": "error", "reason": "Failed to prepare transaction {}"}},)",
            alpm_strerror(*alpm_errno)
        );
        return false;
    }

    /* Actually perform removal */

    if (alpm_trans_commit(handle, &data) < 0) {
        std::print(
    R"({{"type": "error", "reason": "Failed to commit transaction {}"}},)",
            alpm_strerror(*alpm_errno)
        );
        return false;
    }

    alpm_list_free(data);

    if (alpm_trans_release(handle) < 0) {
        std::print(
    R"({{"type": "error", "reason": "Failed to release transaction {}"}},)",
            alpm_strerror(*alpm_errno)
        );
        return false;
    }
    return true;
}


void remove_pkg(
    alpm_handle_t                  *handle,
    alpm_db_t                      *local_db,
    const alpm_errno_t             *alpm_errno,
    const std::vector<std::string> &pkgs
)
{
    for (const auto &pkg_name : pkgs) {
        alpm_pkg_t *pkg = alpm_db_get_pkg(local_db, pkg_name.c_str());

        if (pkg == nullptr) {
            std::print(
R"({{"type": "warn", "reason": "Failed to get package {} from local db."}},)",
                alpm_strerror(*alpm_errno)
            );
            continue;
        }

        if (alpm_remove_pkg(handle, pkg) < 0) {
            std::print(
                R"({{"type": "warn", "reason": "Failed to remove {}: {}"}},)",
                pkg_name, alpm_strerror(*alpm_errno)
            );
        }
    }
}