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
#include <json/value.h>
#ifndef CONFIG_HH__
#define CONFIG_HH__

#include <optional>
#include <memory>

class ArgParser;
class Logger;


/**
 * @class Config
 * @brief A class dedicated to managing config and caching of json objects.
 */
class Config
{
public:
    Config(
        const std::shared_ptr<Logger> &logger,
        const std::shared_ptr<ArgParser> &arg_parser
    );

    /**
     * @brief Loads a config from the configuration file.
     * @returns a Json::Value on success, or an std::nullopt on failure.
     */
    [[nodiscard]]
    auto load(
        bool from_file = false, bool return_val = true
    ) -> std::optional<Json::Value>;

    /**
     * @brief Saves a config from the configuration file.
     * @returns true on success, or false on failure
     */
    auto save(const Json::Value &config) -> bool;

private:
    std::shared_ptr<ArgParser> m_arg_parser;
    std::shared_ptr<Logger>    m_logger;

    std::string m_config_path;
    std::string m_cache_path;

    std::string m_home;
    std::string m_xdg_cache_home;
    std::string m_xdg_config_home;

    bool m_should_cache;

    Json::Value m_cache;
    Json::Value m_config;

    /**
     * @brief Searches for config path.
     * @returns true on success, or false on failure.
     */
    auto search_config_path() -> bool;

    /**
     * @brief Searches for config path in the cli arg.
     * @returns true on found, or false on not found.
     */
    auto search_config_from_arg() -> bool;

    /**
     * @brief Searches for cache path.
     * @returns true on success, or false on failure.
     */
    auto search_cache_path() -> bool;
};

#endif /* config.hh */