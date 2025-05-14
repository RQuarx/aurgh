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
    m_alpm_errno(ALPM_ERR_OK)
{
    if (arg_parser->option_arg(m_root_path, { "-r", "--root" })) {
        if (!std::filesystem::exists(m_root_path)) {
            m_logger->log(
                Logger::Error,
                "Root path {} not a valid path.",
                m_root_path
            );
            exit(EXIT_FAILURE);
        }

        if (!std::filesystem::is_directory(m_root_path)) {
            m_logger->log(
                Logger::Error,
                "Root path {} is not a directory.",
                m_root_path
            );
            exit(EXIT_FAILURE);
        }
    } else { m_root_path = DEFAULT_ROOT_PATH; }

    if (arg_parser->option_arg(m_db_path, { "-d", "--db-path" })) {
        if (!std::filesystem::exists(m_db_path)) {
            m_logger->log(
                Logger::Error,
                "Database path {} not a valid path.",
                m_db_path
            );
            exit(EXIT_FAILURE);
        }

        if (!std::filesystem::is_directory(m_db_path)) {
            m_logger->log(
                Logger::Error,
                "Database path {} is not a directory.",
                m_db_path
            );
            exit(EXIT_FAILURE);
        }
    } else { m_db_path = DEFAULT_DB_PATH; }

    m_alpm_handle = alpm_initialize(
        m_root_path.c_str(), m_db_path.c_str(), &m_alpm_errno
    );

    if (m_alpm_handle == nullptr) {
        m_logger->log(
            Logger::Error,
            "Failed to initialize alpm: {}",
            alpm_strerror(m_alpm_errno)
        );
        exit(m_alpm_errno);
    }

    if (arg_parser->option_arg(m_helper_path, { "-H", "--helper-path" })) {
        if (!std::filesystem::exists(m_helper_path)) {
            m_logger->log(
                Logger::Error,
                "Helper path {} not a valid path.",
                m_helper_path
            );
            exit(EXIT_FAILURE);
        }

        if (!std::filesystem::is_regular_file(m_helper_path)) {
            m_logger->log(
                Logger::Error,
                "Helper path {} is not a file.",
                m_helper_path
            );
            exit(EXIT_FAILURE);
        }

        if (access(m_helper_path.c_str(), X_OK) != 0) {
            m_logger->log(
                Logger::Error,
                "Helper path {} is not an executable.",
                m_helper_path
            );
            exit(EXIT_FAILURE);
        };
    } else { m_helper_path = DEFAULT_HELPER_PATH; }

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
    Json::CharReaderBuilder reader;
    Json::Value             data;
    std::string             errs;
    if (!Json::parseFromStream(reader, iss, &data, &errs)) {
        m_logger->log(
            Logger::Error,
            "Failed to parse Json from stream: {}", errs
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
Client::install(
    const std::string &pkg_name, const std::string &prefix) -> bool
{
    std::string url = std::format("https://aur.archlinux.org/{}.git", pkg_name);
    Process git("git", { "clone", url }, m_logger, prefix);

    while (git.is_done() < 0) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (git.is_done() == std::nullopt) return false;
    git.kill();

    if (!std::filesystem::exists(pkg_name)) {
        m_logger->log(Logger::Error, "Failed to git clone.");
        return false;
    }

    chdir(pkg_name.c_str());

    Process ls("ls", {}, m_logger, prefix + pkg_name);
    return false;
}


auto
Client::get_search_by_keywords(
    ) -> std::vector<std::string>
{
    return {
        "name", "name-desc",
        "depends", "checkdepends",
        "optdepends", "makedepends",
        "maintainer", "submitter",
        "provides", "conflicts",
        "replaces", "keywords",
        "groups", "comaintainers"
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
    std::string args{
        "pkexec "     +
        m_helper_path +
        " remove "    +
        m_root_path   +
        " "           +
        m_db_path
    };

    for (const auto &pkg : pkgs) {
        args += " " + pkg;
    }

    auto res = utils::run_command(args);
    m_logger->log(Logger::Debug, "{}", res->first);

    if (res->second != EXIT_SUCCESS) {
        m_logger->log(
            Logger::Error,
            "{}",
            res->first
        );
        return false;
    }

    return true;
}


void
Client::remove_pkg(
    const std::vector<std::string> &pkgs,
    alpm_db_t                      *local_db
)
{
    for (const auto &pkg_name : pkgs) {
        alpm_pkg_t *pkg = alpm_db_get_pkg(local_db, pkg_name.c_str());

        if (pkg == nullptr) {
            m_logger->log(
                Logger::Warn,
                "Failed to get package {} from local database",
                pkg_name
            );
            continue;
        }

        if (alpm_remove_pkg(m_alpm_handle, pkg) < 0) {
            m_logger->log(
                Logger::Error,
                "Failed to remove package {}: {}",
                pkg_name, alpm_strerror(m_alpm_errno)
            );
        }
    }
}