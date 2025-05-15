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
#include <alpm.h>

namespace Json { class Value; }
class ArgParser;
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
         * @param url A custom AUR url, defaults to https://aur.archlinux.org/rpc/v5
         */
        explicit Client(
            const std::shared_ptr<Logger>    &logger,
            const std::shared_ptr<ArgParser> &arg_parser,
            std::string_view                  url = ""
        );


        ~Client();


        /**
         * @brief Searches a package on the AUR.
         * @param args the args that is given to url/search/...
         * @param by What to search by.
         * @returns A Json::Value object.
         */
        auto search(
            const std::string &args,
            const std::string &by = ""
        ) -> Json::Value;


        /**
         * @brief Gets an information about a package.
         * @param args the args that is given to url/info?arg[]=...
         * @returns A Json::Value object.
         */
        auto info(const std::string &args) -> Json::Value;


        /**
         * @brief Installs an aur package
         * @param pkgs The name of the packages to be removed.
         * @returns true on success, or false on failure
         */
        auto install(const std::vector<std::string> &pkgs) -> bool;


        /**
         * @brief Removes an aur package
         * @param pkgs The name of the packages to be removed.
         * @returns true on success, or false on failure
         */
        auto remove(const std::vector<std::string> &pkgs) -> bool;


        /**
         * @brief Returns packages from the alpm local database.
         * @return A vector of alpm_pkg_t*.
         */
        auto get_installed_pkgs() -> std::vector<alpm_pkg_t*>;

        /**
         * @brief Get the available "search by" keywords used for the search function.
         * @returns an array of const std::string with the size of 14.
         */
        static auto get_search_by_keywords() -> std::vector<std::string>;

        /**
         * @brief Get the available "sort by" keywords used for the search function.
         * @returns an array of const std::string with the size of 7.
         */
        static auto get_sort_by_keywords() -> std::vector<std::string>;

    private:
        static const inline std::string DEFAULT_AUR_URL =
            "https://aur.archlinux.org/rpc/v5";
        static const inline std::string DEFAULT_HELPER_PATH =
            "/usr/share/aurgh/helper";

        static const constexpr char *DEFAULT_ROOT_PATH  = "/";
        static const constexpr char *DEFAULT_DB_PATH    = "/var/lib/pacman";

        std::string_view           m_url;
        std::shared_ptr<Logger>    m_logger;
        std::shared_ptr<ArgParser> m_arg_parser;

        std::string m_helper_path;
        std::string m_prefix_path;
        std::string m_root_path;
        std::string m_db_path;

        alpm_errno_t            m_alpm_errno;
        alpm_handle_t          *m_alpm_handle{};

        /**
         * @brief Initialize root, database, or helper path.
         * @param t Which path to initialize. (0 - root, 1 - db, 2 - helper)
         * @return A valid string of path, or an empty string on failure.
         */
        auto initialize_path(uint8_t t) -> std::string;


        /**
         * @brief Parses Json::Value from @p iss .
         * @return A valid Json::Value or an empty Json::Value on failure.
         */
        auto get_json_from_stream(
            std::istringstream &iss
        ) -> Json::Value;


        /**
         * @brief Gets the locality of a package.
         * @param pkg_name The name of the package.
         * @param sync_dbs A list of sync databases.
         * @return 0 on native locality, 1 on foreign locality
         */
        static auto get_pkg_locality(
            const std::string &pkg_name,
            alpm_list_t       *sync_dbs
        ) -> uint8_t;
    };
} /* namespace AUR */

#endif /* aur_client.hh */