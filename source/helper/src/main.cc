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
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <print>

#include <json/reader.h>
#include <json/value.h>

#include "alpm.hh"


auto
main(int32_t argc, char **argv) -> int32_t
{
    if (getuid() != 0) {
        std::println("This program must be run as superuser!");
        return EXIT_FAILURE;
    }

    std::vector<std::string> args{ argv, argv + argc };

    if (args.at(1) == "help") {
        std::println("Usage: {} <prefix-path>", args.at(0));
    }



    std::string prefix_path         { args.at(1) };
    std::string operation_file_path { prefix_path + "/operation.json" };
    Json::Value operation           { Json::objectValue };
    std::ifstream operation_file    { operation_file_path };

    operation_file >> operation;
    operation_file.close();

    std::string  type      { operation["operation"].asString() };
    std::string  root_path { operation["root"].asString() };
    std::string  db_path   { operation["db-path"].asString() };
    alpm_errno_t err       { ALPM_ERR_OK };
    Alpm         alpm      { root_path, db_path, err };

    std::vector<std::string> pkgs;
    pkgs.reserve(operation["pkgs"].size());

    for (const auto &p : operation["pkgs"]) {
        pkgs.push_back(p.asString());
    }

    std::filesystem::remove(operation_file_path);

    if (err != ALPM_ERR_OK) return static_cast<int32_t>(err);

    if (type == "remove") {
        bool _ = alpm.remove_packages(pkgs);
        return static_cast<int32_t>(err);
    }

    if (type == "install") {

        return EXIT_SUCCESS;
    }
}