
#include "config.hh"
#include "utils.hh"


namespace
{
    template<typename... T_Args>
    auto a_format( std::string_view fmt, T_Args &&...args ) -> std::string
    {
        std::string str { std::vformat(fmt, std::make_format_args(args...)) };
        return str;
    }


    auto
    get_env( const std::string &p_str ) -> std::string
    {
        const char *res { std::getenv(p_str.c_str()) };
        return res == nullptr ? "" : res;
    }
}


auto
split_string( const std::string &str, const size_t &idx ) -> str_pair
{
    if (idx == 0 || idx == std::string::npos)
        return { str, "" };
    auto a { str.substr(0,  idx) };
    auto b { str.substr(idx + 1) };
    return { a, b };
}


auto
get_prefix_path( void ) -> std::string
{
    std::string result { "" };

    for (size_t i { 0 }; i < PREFIX_PATH.length(); i++) {
        if (PREFIX_PATH.at(i) == '$' && PREFIX_PATH.at(i + 1) == '{') {
            size_t end_idx { i + 2 };
            while (PREFIX_PATH.at(end_idx) != '}') end_idx++;
            std::string var { PREFIX_PATH.substr(i + 2, end_idx - 2) };

            result += get_env(var.c_str());

            i += end_idx;
            continue;
        }

        result += PREFIX_PATH.at(i);
    }

    return result;
}