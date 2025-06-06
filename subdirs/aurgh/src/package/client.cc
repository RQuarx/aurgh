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

#include <json/reader.h>
#include <json/json.h>
#include <alpm.h>

#include "package/client.hh"
#include "config.hh"
#include "logger.hh"
#include "utils.hh"
#include "data.hh"

using pkg::Client;


Client::Client(str_view url) :
    m_config(data::config->get_config()),
    m_helper_path(initialize_path(2)),
    m_prefix_path(initialize_path(3)),
    m_root_path(initialize_path(0)),
    m_db_path(initialize_path(1)),
    m_alpm_errno(ALPM_ERR_OK),
    m_alpm_handle(alpm_initialize(
        m_root_path.c_str(), m_db_path.c_str(), &m_alpm_errno
    ))
{
    if (!url.empty()) { m_url = url; }
    else { m_url = (*m_config)["links"]["aur-url"].asString(); }

    m_pkexec = (*m_config)["app"]["pkexec-binary"].asString();

    if (m_alpm_handle == nullptr) {
        data::logger->log(
            Logger::Error,
            "Failed to initialize alpm: {}",
            alpm_strerror(m_alpm_errno)
        );
        exit(m_alpm_errno);
    }

    data::logger->log(
        Logger::Debug, "AUR Client instance successfully created"
    );
}


Client::~Client()
{ alpm_release(m_alpm_handle); }


auto
Client::get_json_from_stream(std::istringstream &iss) -> json
{
    json data;

    try {
        iss >> data;
    } catch (const std::exception &e) {
        data::logger->log(
            Logger::Error,
            "Failed to parse json from istringstream: {}",
            e.what()
        );
    }

    return data;
}


auto
Client::search(const str &args, const str &by) -> json
{
    str full_url = std::format("{}/search/{}", m_url, args);
    if (!by.empty()) {
        full_url.append("?by=" + by);
    }

    str read_buffer;

    if (
        utils::perform_curl(nullptr, full_url, read_buffer) == CURLE_FAILED_INIT
    ) data::logger->log(Logger::Error, "Failed to initialise CURL.");

    std::istringstream iss(read_buffer);
    return get_json_from_stream(iss);
}


auto
Client::info(const str &args) -> json
{
    str full_url = std::format("{}/info?arg[]={}", m_url, args);

    str read_buffer;
    if (
        utils::perform_curl(nullptr, full_url, read_buffer) == CURLE_FAILED_INIT
    ) data::logger->log(Logger::Error, "Failed to initialise CURL.");

    std::istringstream iss(read_buffer);
    return get_json_from_stream(iss);
}


auto
Client::install(const vec<str> &pkgs) -> bool
{
    json root { Json::objectValue };

    root["operation"]      = "install";
    root["install-prefix"] = m_prefix_path;
    root["root"]           = m_root_path;
    root["db-path"]        = m_db_path;
    root["pkgs"]           = Json::arrayValue;

    for (const auto &pkg : pkgs) { root["pkgs"].append(pkg); }

    data::logger->log(
        Logger::Info, "Installing: {}", root["pkgs"].toStyledString()
    );

    std::ofstream operation_file(m_prefix_path + "/operation.json");
    operation_file << root;
    operation_file.close();

    auto res = utils::run_command(std::format(
        "{} {} --prefix {}",
        m_pkexec,
        m_helper_path,
        m_prefix_path
    ));

    if (res->second != ALPM_ERR_OK) {
        data::logger->log(
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
        data::logger->log(
            Logger::Error,
            "Failed to initialize alpm: {}",
            alpm_strerror(m_alpm_errno)
        );
        return false;
    }

    return true;
}


auto
Client::get_search_by_keywords() -> vec<str>
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
Client::get_sort_by_keywords() -> vec<str>
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
Client::get_locally_installed_pkgs() -> vec<alpm_pkg_t*>
{
    vec<alpm_pkg_t*> pkgs;

    alpm_db_t   *local_db = alpm_get_localdb(m_alpm_handle);
    alpm_list_t *pkg_list = alpm_db_get_pkgcache(local_db);

    if (pkg_list == nullptr) {
        data::logger->log(
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
Client::get_installed_pkgs() -> vec<alpm_pkg_t*>
{
    vec<alpm_pkg_t*> pkgs;

    alpm_db_t   *local_db = alpm_get_localdb(m_alpm_handle);
    alpm_list_t *pkg_list = alpm_db_get_pkgcache(local_db);

    if (pkg_list == nullptr) {
        data::logger->log(
            Logger::Warn,
            "Failed to retrieve package list"
        );

        return pkgs;
    }

    alpm_list_t *sync_dbs = alpm_get_syncdbs(m_alpm_handle);

    for (auto *node = pkg_list; node != nullptr; node = node->next) {
        auto       *pkg      = static_cast<alpm_pkg_t*>(node->data);
        const char *pkg_name = alpm_pkg_get_name(pkg);

        if (get_pkg_locality(pkg_name, sync_dbs) == 0) pkgs.push_back(pkg);
    }

    return pkgs;
}


auto
Client::get_pkg_locality(
    const str   &pkg_name,
    alpm_list_t *sync_dbs
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
Client::remove(const vec<str> &pkgs) -> bool
{
    json root { Json::objectValue };

    root["operation"] = "remove";
    root["root"]      = m_root_path;
    root["db-path"]   = m_db_path;
    root["pkgs"]      = Json::arrayValue;

    for (const auto &pkg : pkgs) { root["pkgs"].append(pkg); }

    data::logger->log(Logger::Info, "Removing: {}", root["pkgs"].toStyledString());

    std::ofstream operation_file(m_prefix_path + "/operation.json");
    operation_file << root;
    operation_file.close();

    auto res = utils::run_command(std::format(
        "{} {} --prefix {}",
        m_pkexec,
        m_helper_path,
        m_prefix_path
    ));

    if (res->second != ALPM_ERR_OK) {
        data::logger->log(
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
        data::logger->log(
            Logger::Error,
            "Failed to initialize alpm: {}",
            alpm_strerror(m_alpm_errno)
        );
        return false;
    }

    return true;
}


auto
Client::initialize_path(uint8_t t) -> str
{
    using utils::expand_envs;

    switch (t) {
    case 0:  return expand_envs((*m_config)["paths"]["root-path"].asString());
    case 1:  return expand_envs((*m_config)["paths"]["db-path"].asString());
    case 2:  return expand_envs((*m_config)["paths"]["helper-path"].asString());
    case 3:  return expand_envs((*m_config)["paths"]["prefix-path"].asString());
    default: return "";
    }
}


auto
Client::find_pkg(const str &name) -> alpm_pkg_t*
{
    alpm_db_t *local_db = alpm_get_localdb(m_alpm_handle);
    return alpm_db_get_pkg(local_db, name.c_str());
}