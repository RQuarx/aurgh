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

#include "arg_parser.hh"
#include "alpm.hh"
#include "log.hh"


auto
main(int32_t argc, char **argv) -> int32_t
{
    if (getuid() != 0) {
        std::println("This program must be run as superuser!");
        return EXIT_FAILURE;
    }

    ArgParser arg_parser { argc, argv };

    arg_parser
        .add_options(
            ArgInput({ "-p", "--prefix" }, "Specify the prefix path", "path"))
        .parse();

    if (arg_parser.get_option("prefix").empty()) {
        std::println("Prefix path is empty!");
        return EXIT_FAILURE;
    }

    std::string prefix_path         { arg_parser.get_option("prefix") };
    std::string operation_file_path { prefix_path + "/operation.json" };
    Json::Value operation           { Json::objectValue };
    std::ifstream operation_file    { operation_file_path };

    operation_file >> operation;
    operation_file.close();

    std::string          type      { operation["operation"].asString() };
    std::string          root_path { operation["root"].asString() };
    std::string          db_path   { operation["db-path"].asString() };
    alpm_errno_t         err       { ALPM_ERR_OK };
    std::shared_ptr<Log> log       { std::make_shared<Log>(prefix_path) };
    Alpm                 alpm      {
        root_path, db_path, prefix_path, log, err
    };

    using std::filesystem::remove;


    if (err != ALPM_ERR_OK) {
        log->write(true, "Failed to initialize alpm: {}", alpm_strerror(err));
        return remove(operation_file_path), 1;
    };

    if (type == "remove") {
        std::vector<std::string> pkgs;
        pkgs.reserve(operation["pkgs"].size());

        for (const auto &p : operation["pkgs"]) {
            pkgs.push_back(p.asString());
        }

        bool _ = alpm.remove_packages(pkgs);
        return remove(operation_file_path), 1;
    }

    if (type == "install") {
        bool _ = alpm.download_and_install_package(operation["pkg"].asString());
        return remove(operation_file_path), 1;
    }

    return remove(operation_file_path), 0;
}