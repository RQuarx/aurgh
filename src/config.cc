#include <filesystem>

#include <json/reader.h>
#include <json/writer.h>

#include "arg_parser.hh"
#include "config.hh"
#include "logger.hh"
#include "utils.hh"


Config::Config(
    const std::shared_ptr<Logger> &logger,
    const std::shared_ptr<ArgParser> &arg_parser
) :
    m_arg_parser(arg_parser),
    m_logger(logger),
    m_home(utils::get_env("HOME")),
    m_xdg_cache_home(utils::get_env("XDG_CACHE_HOME")),
    m_xdg_config_home(utils::get_env("XDG_CONFIG_HOME")),
    m_should_cache(m_arg_parser->find_arg({ "", "--no-cache" }))
{
    if (m_xdg_cache_home.empty()) {
        m_xdg_cache_home = std::format("{}/.cache", m_home);
    }

    if (m_xdg_config_home.empty()) {
        m_xdg_config_home = std::format("{}/.config", m_home);
    }

    if (!search_config_path())                   exit(EXIT_FAILURE);
    if (m_should_cache && !search_cache_path())  exit(EXIT_FAILURE);
}


auto
Config::search_config_path() -> bool
{
    using std::filesystem::create_directory;
    using std::filesystem::exists;
    using std::filesystem::path;

    if (!search_config_from_arg()) {
        m_config_path = std::format("{}/aurgh/aurgh.json", m_xdg_config_home);
        if (exists(m_config_path)) return true;

        std::error_code error_code;

        if (!create_directory(path( m_config_path ).parent_path(), error_code)) {
            m_logger->log(
                Logger::Error,
                "Failed to create config directory: {}",
                error_code.message()
            );
            return false;
        }

        std::ofstream p { m_config_path };
        return true;
    }

    return true;
}


auto
Config::search_config_from_arg() -> bool
{
    using std::filesystem::create_directory;
    using std::filesystem::is_regular_file;
    using std::filesystem::exists;
    using std::filesystem::path;

    std::string option;
    if (!m_arg_parser->option_arg(option, { "-c", "--config" })) {
        return false;
    }

    if (!exists(option)) {
        m_logger->log(
            Logger::Warn,
            "Provided config file does not exist, using default config path."
        );

        return false;
    }

    if (!is_regular_file(option)) {
        m_logger->log(
            Logger::Warn,
            "Provided config option was not a file, using default config path."
        );

        return false;
    }

    path        config_path { option };
    std::string extension   { config_path.extension() };

    if (extension != ".json" && extension != ".jsonc") {
        m_logger->log(
            Logger::Warn,
            "Provided config file isnt a json type, using default config path."
        );

        return false;
    }

    m_config_path = config_path.string();
    return true;
}


auto
Config::search_cache_path() -> bool
{
    using std::filesystem::create_directory;
    using std::filesystem::exists;
    using std::filesystem::path;

    m_cache_path = std::format("{}/aurgh/aurgh.json", m_xdg_cache_home);
    if (exists(m_cache_path)) return true;

    std::error_code error_code;

    if (!create_directory(path(m_cache_path).parent_path(), error_code)) {
        m_logger->log(
            Logger::Error,
            "Failed to create cache directory: {}",
            error_code.message()
        );
        return false;
    }

    std::ofstream p { m_cache_path };
    return true;
}


auto
Config::load(bool from_file, bool return_val) -> std::optional<Json::Value>
{
    if (!m_config.empty() && !from_file && return_val) return m_config;

    std::ifstream config_file;

    try {
        config_file.open(m_config_path);
    } catch (const std::exception &e) {
        m_logger->log(
            Logger::Error,
            "Failed to open config file: {}, {}",
            m_config_path, e.what()
        );
        return std::nullopt;
    }

    try {
        Json::Reader reader;
        reader.parse(config_file, m_config);
    } catch (const std::exception &e) {
        m_logger->log(
            Logger::Warn,
            "Failed to parse json file: {}",
            e.what()
        );
        return std::nullopt;
    }

    if (return_val) return m_config;
    return std::nullopt;
}


auto
Config::save(const Json::Value &config) -> bool
{
    std::ofstream config_file;

    try {
        config_file.open(m_config_path);
    } catch (const std::exception &e) {
        m_logger->log(
            Logger::Error,
            "Failed to open config file: {}, {}",
            m_config_path, e.what()
        );
        return false;
    }

    config_file << config;

    return load(true, false) == std::nullopt;
}