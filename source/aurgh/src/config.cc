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

#include <json/reader.h>
#include <json/writer.h>

#include "arg_parser.hh"
#include "config.hh"
#include "logger.hh"
#include "utils.hh"


Config::Config(
    const std::shared_ptr<Logger>    &logger,
    const std::shared_ptr<ArgParser> &arg_parser
) :
    m_arg_parser(arg_parser),
    m_logger(logger),
    m_config(std::make_shared<Json::Value>(Json::objectValue)),
    m_cache(std::make_shared<Json::Value>(Json::objectValue))
{
    using std::filesystem::is_regular_file;
    using std::filesystem::exists;

    auto valid_file = [](const std::string &path){
        return exists(path) && is_regular_file(path);
    };

    bool arg_config = false;

    if (arg_parser->option_arg(m_config_path, { "-c", "--config" })) {
        arg_config = true;
    }

    if (!arg_config) {
        m_config_path = std::format(
            "{}/.config/aurgh/config.json",
            utils::get_env("HOME")
        );
    }

    m_cache_path = std::format(
        "{}/.cache/aurgh/cache.json",
        utils::get_env("HOME")
    );

    if (!valid_file(m_config_path)) {
        m_logger->log(
            Logger::Warn,
            "Config file {} doesn't exist! Using systemwide config path: {}",
            m_config_path, GLOBAL_CONFIG_PATH
        );

        m_config_path = GLOBAL_CONFIG_PATH;
    }

    if (!load_config()) {
        throw std::runtime_error("Invalid config file");
    }

    if (!load_cache()) {
        throw std::runtime_error("Invalid config file");
    }
}


auto
Config::save() -> bool
{
    using std::filesystem::path;

    path cache_path  = m_cache_path;

    if (!cache_path.parent_path().string().empty() &&
        !std::filesystem::exists(cache_path.parent_path())) {
        try {
            std::filesystem::create_directory(cache_path.parent_path());
        } catch (const std::exception &e) {
            m_logger->log(
                Logger::Error,
                "Failed to create directory {}: {}",
                cache_path.parent_path().string(), e.what()
            );
            return false;
        }
    }

    std::ofstream file;

    try {
        file.open(m_cache_path);
    } catch (const std::exception &e) {
        m_logger->log(
            Logger::Error,
            "Failed to open file {}: {}",
            m_cache_path, e.what()
        );
        return false;
    }

    try {
        file << *m_cache;
    } catch (const std::exception &e) {
        m_logger->log(
            Logger::Error,
            "Failed to write to file {}: {}",
            m_cache_path, e.what()
        );
        return false;
    }

    m_logger->log(
        Logger::Debug,
        "Writting to cache file: {}",
        m_cache_path
    );

    return true;
}


auto
Config::get_config() -> std::shared_ptr<Json::Value>
{ return m_config; }


auto
Config::get_cache() -> std::shared_ptr<Json::Value>
{ return m_cache; }


auto
Config::load_config() -> bool
{
    std::ifstream config_file;

    try {
        config_file.open(m_config_path);
    } catch (const std::exception &e) {
        m_logger->log(
            Logger::Error,
            "Failed to read config file {}: {}",
            m_config_path, e.what()
        );
        return false;
    }


    try {
        config_file >> *m_config;
    } catch (const std::exception &e) {
        m_logger->log(
            Logger::Error,
            "Failed to parse json from config file {}: {}",
            m_config_path, e.what()
        );
        return false;
    }

    return !m_config->empty();
}


auto
Config::load_cache() -> bool
{
    std::ifstream cache_file;

    try {
        cache_file.open(m_cache_path);
    } catch (const std::exception &e) {
        return true;
    }

    if (cache_file.peek() == std::ifstream::traits_type::eof()) {
        return true;
    }

    try {
        cache_file >> *m_cache;
    } catch (const std::exception &e) {
        m_logger->log(
            Logger::Error,
            "Failed to parse json from cache file {}: {}",
            m_cache_path, e.what()
        );
        return false;
    }

    return !m_cache->empty();
}