/**
 * @file aur_client.cc
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

#include <json/json.h>

#include "aur_client.hh"
#include "logger.hh"


AUR_Client::AUR_Client(Logger *logger, std::string_view url) :
    m_logger(logger)
{
    if (!url.empty()) m_url = url;

    m_logger->log(
        Logger::Debug, "AUR Client instance successfully created"
    );
}


auto
AUR_Client::perform_curl(
    const std::string &url, std::string &read_buffer
) -> CURLcode
{
    CURL *curl = curl_easy_init();

    if (curl == nullptr) {
        m_logger->log(Logger::Error, "Failed to initialise CURL.");
        return CURLE_FAILED_INIT;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Utils::write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return res;
}


auto
AUR_Client::get_json_from_stream(std::istringstream &iss) -> Json::Value
{
    Json::CharReaderBuilder reader;
    Json::Value data;
    std::string errs;
    if (!Json::parseFromStream(reader, iss, &data, &errs)) {
        m_logger->log(
            Logger::Error,
            "Failed to parse Json from stream: {}", errs
        );
    }

    return data;
}


auto
AUR_Client::search(
    const std::string &args, const std::string &by) -> Json::Value
{
    std::string full_url = std::format("{}/search/{}", m_url, args);
    if (!by.empty()) {
        full_url.append("?by=" + by);
    }

    std::string read_buffer;
    perform_curl(full_url, read_buffer);

    std::istringstream iss(read_buffer);
    return get_json_from_stream(iss);
}


auto
AUR_Client::info(const std::string &args) -> Json::Value
{
    std::string full_url = std::format("{}/info?arg[]={}", m_url, args);

    std::string read_buffer;
    perform_curl(full_url, read_buffer);

    std::istringstream iss(read_buffer);
    return get_json_from_stream(iss);
}


auto
AUR_Client::get_search_by_keywords(
    ) -> std::array<const std::string, SEARCH_BY_KEYWORDS>
{
    return {{
        "name", "name-desc",
        "depends", "checkdepends",
        "optdepends", "makedepends",
        "maintainer", "submitter",
        "provides", "conflicts",
        "replaces", "keywords",
        "groups", "comaintainers"
    }};
}


auto
AUR_Client::get_sort_by_keywords(
    ) -> std::array<const std::string, SORT_BY_KEYWORDS>
{
    return {{
        "Name",
        "NumVotes",
        "Popularity",
        "Maintainer",
        "LastModified"
    }};
}