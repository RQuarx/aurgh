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

#include "aur_client.hh"
#include "logger.hh"
#include "process.hh"

using AUR::Client;


Client::Client(const std::shared_ptr<Logger> &logger, std::string_view url) :
    m_url(!url.empty() ? url : DEFAULT_AUR_URL),
    m_logger(logger)
{
    m_logger->log(
        Logger::Debug, "AUR Client instance successfully created"
    );
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

    while (git.is_done() == -1) {
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