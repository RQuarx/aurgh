#include <format>

#include <gtkmmconfig.h>
#include <json/reader.h>
#include <json/value.h>
#include <curl/curl.h>
#include <alpm.h>

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
}


namespace utils
{
    auto
    get_env( const std::string &p_key ) -> std::string
    {
        const char *res { std::getenv(p_key.c_str()) };
        return res == nullptr ? "" : res;
    }


    auto
    get_prefix_path() -> std::string
    {
        std::string result;

        for (size_t i { 0 }; i < PREFIX_PATH.length(); i++) {
            if (PREFIX_PATH.at(i) == '$' && PREFIX_PATH.at(i + 1) == '{') {
                size_t end_idx { i + 2 };
                while (PREFIX_PATH.at(end_idx) != '}') end_idx++;
                std::string var { PREFIX_PATH.substr(i + 2, end_idx - 2) };

                result += get_env(var);

                i += end_idx;
                continue;
            }

            result += PREFIX_PATH.at(i);
        }

        return result;
    }


    void
    init_versions( void )
    {
        VERSIONS["gtkmm"] = std::format("{}.{}.{}", GTKMM_MAJOR_VERSION,
                                                    GTKMM_MINOR_VERSION,
                                                    GTKMM_MICRO_VERSION);
        VERSIONS["libcurl"] = curl_version_info(CURLVERSION_NOW)->version;
        VERSIONS["jsoncpp"] = JSONCPP_VERSION_STRING;
        VERSIONS["libalpm"] = alpm_version();
    }
}

namespace Json
{
    auto
    from_string( const std::string &p_str ) -> Json::Value
    {
        std::istringstream iss { p_str };
        Json::Value root;

        try {
            iss >> root;
        } catch (const std::exception &e) {
            throw std::runtime_error(e.what());
        }
        return root;
    }
}