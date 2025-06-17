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
#ifndef __CONFIG_HH__
#define __CONFIG_HH__

#include <json/value.h>
#include "types.hh"

class ArgParser;
class Logger;


/**
 * @class Config
 * @brief A class dedicated to managing config and caching of json objects.
 */
class Config
{
public:
    Config();


    /**
     * @brief Retrieves the loaded configuration data.
     * @return A shared pointer to a Json::Value representing the configuration.
     */
    [[nodiscard]]
    auto get_config() -> shared_ptr<json>;


    /**
     * @brief Retrieves the loaded cache data.
     * @return A shared pointer to a Json::Value representing the cache.
     */
    [[nodiscard]]
    auto get_cache() -> shared_ptr<json>;


    /**
     * @brief Saves the config and cache to disk.
     * @return true on success, or false on failure.
     */
    auto save() -> bool;

private:
    str_view GLOBAL_CONFIG_PATH = "/etc/aurgh/config.jsonc";

    shared_ptr<json> m_config;
    shared_ptr<json> m_cache;

    str m_config_path;
    str m_cache_path;


    /**
     * @brief Loads the config from file to memory.
     * @return true on success, or false on failure.
     */
    auto load_config() -> bool;


    /**
     * @brief Loads the cache from file to memory.
     * @return true on success, or false on failure.
     */
    auto load_cache() -> bool;
};

#endif /* __CONFIG_HH__ */