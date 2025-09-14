#pragma once
#include <unordered_map>
#include <expected>
#include <format>
#include <string>

namespace Json { class Value; }


namespace utils
{
    inline std::unordered_map<std::string, std::string> VERSIONS;


    template<typename T_Err>
    auto unexpected( T_Err err ) -> std::unexpected<T_Err>
    { return std::unexpected<T_Err>(err); }


    template<typename... T_Args>
    auto unexpected( std::string_view fmt,
                     T_Args      &&...args ) -> std::unexpected<std::string>
    {
        std::string msg { std::vformat(fmt, std::make_format_args(args...)) };
        return std::unexpected<std::string>(msg);
    }


    auto get_env( const std::string &p_key ) -> std::string;


    /**
     * @brief Returns the PREFIX_PATH from config.hh
     */
    auto get_prefix_path() -> std::string;


    void init_versions();
}


namespace Json
{
    /**
     * @brief A wrapper for converting std::string object to a Json::Value object.
     *
     * @return A Json::Value object.
     */
    auto from_string( const std::string &p_str ) -> Json::Value;
}