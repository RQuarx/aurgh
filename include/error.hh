#pragma once
#include <expected>
#include <format>


namespace err
{
    class error : public std::exception
    {
    public:
        template<typename... T_Args>
        error( std::string_view fmt, T_Args &&...args )
        { msg = std::vformat(fmt, std::make_format_args(args...)); }

        explicit error( const std::string &p_msg ) : msg(p_msg)
        {}

        virtual auto
        what( void ) const noexcept -> const char *
        { return msg.c_str(); }

    private:
        std::string msg;
    };


    class file : public error
    { public: using error::error; };


    class gtk : public error
    { public: using error::error; };


    class init : public error
    { public: using error::error; };


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
} /* namespace err */