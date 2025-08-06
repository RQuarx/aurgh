#pragma once
#include <expected>
#include <format>
#include <string>


namespace utils
{
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


    /**
     * @brief Returns the PREFIX_PATH from config.hh
     */
    auto get_prefix_path( void ) -> std::string;


    template<class T_Exception, typename... T_Args>
    auto fthrow( std::format_string<T_Args...> p_fmt,
                 T_Args                   &&...p_args )
    {
        std::string msg {
            std::vformat(p_fmt, std::make_format_args(p_args...)) };

        throw T_Exception(msg);
    }
}