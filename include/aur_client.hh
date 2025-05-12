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
#ifndef AUR_CLIENT_HH__
#define AUR_CLIENT_HH__

#include <string_view>
#include <memory>
#include <string>
#include <vector>

#include <curl/curl.h>

namespace Json { class Value; }
class Logger;


namespace AUR {
    /**
    * @class Client
    * @brief A class made to fetch json fata from the Arch User Repository.
    */
    class Client
    {
    public:
        /**
        * @brief Constructs an AUR_Client class.
        * @param logger A pointer to a Logger instance.
        * @param url A custom AUR url, defaults to https://aur.archlinux.org/rpc/v5
        */
        explicit Client(
            const std::shared_ptr<Logger> &logger,
            std::string_view               url = ""
        );

        /**
        * @brief Searches a package on the AUR.
        * @param args the args that is given to url/search/...
        * @param by What to search by.
        * @returns A Json::Value object.
        */
        auto search(
            const std::string &args, const std::string &by = "") -> Json::Value;

        /**
        * @brief Gets an information about a package.
        * @param args the args that is given to url/info?arg[]=...
        * @returns A Json::Value object.
        */
        auto info(const std::string &args) -> Json::Value;

        /**
        * @brief Installs an aur package
        * @param pkg_name The name of the package to be installed
        * @param prefix The base directory where the git clones will happen
        * @returns true on success, or false on failure
        */
        auto install(
            const std::string &pkg_name, const std::string &prefix = "") -> bool;

        /**
        * @brief Get the available "search by" keywords used for the search function.
        * @returns an array of const std::string with the size of 14.
        */
        static auto get_search_by_keywords(
            ) -> std::vector<std::string>;

        /**
        * @brief Get the available "sort by" keywords used for the search function.
        * @returns an array of const std::string with the size of 7.
        */
        static auto get_sort_by_keywords(
            ) -> std::vector<std::string>;

    private:
        static const inline std::string_view DEFAULT_AUR_URL =
            "https://aur.archlinux.org/rpc/v5";

        std::string_view        m_url;
        std::shared_ptr<Logger> m_logger;

        auto get_json_from_stream(std::istringstream &iss) -> Json::Value;
    };
} /* namespace AUR */

#endif /* aur_client.hh */