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
#include <chrono>
#include <thread>

#include <json/reader.h>
#include <json/json.h>
#include <alpm.h>

#include "arg_parser.hh"
#include "aur_client.hh"
#include "process.hh"
#include "logger.hh"

using AUR::Client;


Client::Client(
    const std::shared_ptr<Logger>    &logger,
    const std::shared_ptr<ArgParser> &arg_parser,
    std::string_view                  url
) :
    m_url(!url.empty() ? url : DEFAULT_AUR_URL),
    m_logger(logger),
    m_arg_parser(arg_parser),
    m_helper_path(initialize_path(2)),
    m_prefix_path(initialize_path(3)),
    m_root_path(initialize_path(0)),
    m_db_path(initialize_path(1)),
    m_alpm_errno(ALPM_ERR_OK),
    m_alpm_handle(alpm_initialize(
        m_root_path.c_str(), m_db_path.c_str(), &m_alpm_errno
    ))
{
    if (m_alpm_handle == nullptr) {
        m_logger->log(
            Logger::Error,
            "Failed to initialize alpm: {}",
            alpm_strerror(m_alpm_errno)
        );
        exit(m_alpm_errno);
    }

    m_logger->log(
        Logger::Debug, "AUR Client instance successfully created"
    );
}


Client::~Client()
{
    alpm_release(m_alpm_handle);
}


auto
Client::get_json_from_stream(std::istringstream &iss) -> Json::Value
{
    Json::Value data;

    try {
        iss >> data;
    } catch (const std::exception &e) {
        m_logger->log(
            Logger::Error,
            "Failed to parse json from istringstream: {}",
            e.what()
        );
    }

    return data;
}


auto
Client::search(
    const std::string &args, const std::string &by) -> Json::Value
{
    std::string full_url = std::format("{}/search/{}", m_url, args);
    if (!by.empty()) {
        full_url.append("?by=" + by);
    }

    std::string read_buffer;

    if (
        utils::perform_curl(nullptr, full_url, read_buffer) == CURLE_FAILED_INIT
    ) m_logger->log(Logger::Error, "Failed to initialise CURL.");

    std::istringstream iss(read_buffer);
    return get_json_from_stream(iss);
}


auto
Client::info(const std::string &args) -> Json::Value
{
    std::string full_url = std::format("{}/info?arg[]={}", m_url, args);

    std::string read_buffer;
    if (
        utils::perform_curl(nullptr, full_url, read_buffer) == CURLE_FAILED_INIT
    ) m_logger->log(Logger::Error, "Failed to initialise CURL.");

    std::istringstream iss(read_buffer);
    return get_json_from_stream(iss);
}


auto
Client::install(const std::vector<std::string> &pkgs) -> bool
{
    Json::Value root { Json::objectValue };

    root["operation"] = "install";
    root["root"]      = m_root_path;
    root["db-path"]   = m_db_path;
    root["pkgs"]      = Json::arrayValue;

    for (const auto &pkg : pkgs) {
        root["pkgs"].append(pkg);
    }

    m_logger->log(
        Logger::Info, "Installing: {}", root["pkgs"].toStyledString()
    );

    std::ofstream operation_file(m_prefix_path + "/operation.json");
    operation_file << root;
    operation_file.close();

    auto res = utils::run_command(std::format(
        "pkexec {} {}",
        m_helper_path,
        m_prefix_path
    ));

    if (res->second != ALPM_ERR_OK) {
        m_logger->log(
            Logger::Error,
            "Failed to install package{} {}",
            pkgs.size() == 1 ? "" : "s",
            alpm_strerror(alpm_errno_t(res->second))
        );
        return false;
    }

    alpm_release(m_alpm_handle);
    m_alpm_handle = alpm_initialize(
        m_root_path.c_str(), m_db_path.c_str(), &m_alpm_errno
    );

    if (m_alpm_handle == nullptr) {
        m_logger->log(
            Logger::Error,
            "Failed to initialize alpm: {}",
            alpm_strerror(m_alpm_errno)
        );
        return false;
    }

    return true;
}


auto
Client::get_search_by_keywords(
    ) -> std::vector<std::string>
{
    return {
        "name",       "name-desc",
        "depends",    "checkdepends",
        "optdepends", "makedepends",
        "maintainer", "submitter",
        "provides",   "conflicts",
        "replaces",   "keywords",
        "groups",     "comaintainers"
    };
}


auto
Client::get_sort_by_keywords(
    ) -> std::vector<std::string>
{
    return {
        "Name",
        "NumVotes",
        "Popularity",
        "Maintainer",
        "LastModified"
    };
}


auto
Client::get_installed_pkgs() -> std::vector<alpm_pkg_t*>
{
    std::vector<alpm_pkg_t*> pkgs;

    alpm_db_t   *local_db = alpm_get_localdb(m_alpm_handle);
    alpm_list_t *pkg_list = alpm_db_get_pkgcache(local_db);

    if (pkg_list == nullptr) {
        m_logger->log(
            Logger::Warn,
            "Failed to retrieve package list"
        );

        return pkgs;
    }

    alpm_list_t *sync_dbs = alpm_get_syncdbs(m_alpm_handle);

    for (auto *node = pkg_list; node != nullptr; node = node->next) {
        auto       *pkg      = static_cast<alpm_pkg_t*>(node->data);
        const char *pkg_name = alpm_pkg_get_name(pkg);

        if (get_pkg_locality(pkg_name, sync_dbs) == 1) pkgs.push_back(pkg);
    }

    return pkgs;
}


auto
Client::get_pkg_locality(
    const std::string &pkg_name,
    alpm_list_t       *sync_dbs
) -> uint8_t
{
	for (auto *j = sync_dbs; j != nullptr; j = alpm_list_next(j)) {
        auto *alpm_db = static_cast<alpm_db_t *>(j->data);

		if (alpm_db_get_pkg(alpm_db, pkg_name.c_str()) != nullptr) {
			return 0;
		}
	}
	return 1;
}


auto
Client::remove(const std::vector<std::string> &pkgs) -> bool
{
    Json::Value root { Json::objectValue };

    root["operation"] = "remove";
    root["root"]      = m_root_path;
    root["db-path"]   = m_db_path;
    root["pkgs"]      = Json::arrayValue;

    for (const auto &pkg : pkgs) {
        root["pkgs"].append(pkg);
    }

    m_logger->log(Logger::Info, "Removing: {}", root["pkgs"].toStyledString());

    std::ofstream operation_file(m_prefix_path + "/operation.json");
    operation_file << root;
    operation_file.close();

    auto res = utils::run_command(std::format(
        "pkexec {} {}",
        m_helper_path,
        m_prefix_path
    ));

    if (res->second != ALPM_ERR_OK) {
        m_logger->log(
            Logger::Error,
            "Failed to remove package{} {}",
            pkgs.size() == 1 ? "" : "s",
            alpm_strerror(alpm_errno_t(res->second))
        );
        return false;
    }

    alpm_release(m_alpm_handle);
    m_alpm_handle = alpm_initialize(
        m_root_path.c_str(), m_db_path.c_str(), &m_alpm_errno
    );

    if (m_alpm_handle == nullptr) {
        m_logger->log(
            Logger::Error,
            "Failed to initialize alpm: {}",
            alpm_strerror(m_alpm_errno)
        );
        return false;
    }

    return true;
}


auto
Client::initialize_path(uint8_t t) -> std::string
{
    auto log_exit = [this](
        const std::string &msg, const std::string &path
    ){
        m_logger->log(
            Logger::Error,
            std::format("{}", msg),
            path
        );
        exit(EXIT_FAILURE);
    };

    auto check = [this](
        const std::string &path_name,
        const std::string &path
    )
    {
        if (!std::filesystem::exists(path)) {
            m_logger->log(
                Logger::Error,
                "{} path {} not a valid path.", path_name, path
            );
            exit(EXIT_FAILURE);
        }

        if (!std::filesystem::is_directory(path)) {
            m_logger->log(
                Logger::Error,
                "{} path {} not a directory.", path_name, path
            );
            exit(EXIT_FAILURE);
        }
    };

    std::string path;

    if (t == 0) {
        if (!m_arg_parser->option_arg(path, { "-r", "--root" })) {
            return DEFAULT_ROOT_PATH;
        }

        check("Root", path);

    } else if (t == 1) {
        if (!m_arg_parser->option_arg(path, { "-b", "--db-path" })) {
            return DEFAULT_DB_PATH;
        }

        check("Database", path);

    } else if (t == 2) {
        if (!m_arg_parser->option_arg(path, { "", "--helper-path" })) {
            return DEFAULT_HELPER_PATH;
        }

        if (!std::filesystem::exists(path)) {
            log_exit("Helper path {} not a valid path.", path);
        }

        if (!std::filesystem::is_regular_file(path)) {
            log_exit("Helper path {} is not a file.", path);
        }

        if (access(path.c_str(), X_OK) != 0) {
            log_exit("Helper path {} is not an executable.", path);
        }

    } else if (t == 3) {
        if (!m_arg_parser->option_arg(path, { "", "--prefix-path" })) {
            return std::format("{}/.cache/aurgh", utils::get_env("HOME"));
        }

        check("Prefix", path);
    }

    return path;
}