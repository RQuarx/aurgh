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

#include <cstdint>
#include <string>
#include <vector>
#include <print>

#include <alpm.h>

#include "remove.hh"


auto
main(int32_t argc, char **argv) -> int32_t
{
    if (getuid() != 0) {
        std::println("This helper must be run as superuser!");
        return EXIT_FAILURE;
    }

    std::vector<std::string> args{ argv, argv + argc };

    if (args.at(1) == "help") {
        std::println("Usage: {} <options {{args}}>", args.at(0));
        std::println("\nOptions:");
        std::println(
            "\t{:<15}{:<15}{}",
            "install",
            "{paths pkgs}",
            "The first 2 args must be root path and db path."
        );
        std::println(
            "\t{:<15}{:<15}{}",
            "remove",
            "{paths pkgs}",
            "The first 2 args must be root path and db path."
        );
    }

    std::string              root_path = args.at(2);
    std::string              db_path   = args.at(3);
    std::vector<std::string> pkgs{ args.begin() + 4, args.end() };

    alpm_errno_t   alpm_ernno = ALPM_ERR_OK;
    alpm_handle_t *handle     = alpm_initialize(
        root_path.c_str(), db_path.c_str(), &alpm_ernno
    );

    std::print("{{");

    if (args.at(1) == "remove") {
        if (remove(handle, &alpm_ernno, pkgs)) {
            alpm_release(handle);
            std::print(R"({{"type": "success"}}}})");
            return EXIT_SUCCESS;
        }
        alpm_release(handle);
        std::print("}}");
        return EXIT_FAILURE;
    }

    if (args.at(1) == "install") {

        alpm_release(handle);
        return EXIT_SUCCESS;
    }
}