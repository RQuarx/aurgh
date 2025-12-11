#include <format>

#include <alpm.h>
#include <curl/curl.h>
#include <gtkmmconfig.h>
#include <json/version.h>

#include "versions.hh"


namespace { std::unordered_map<std::string, std::string> VERSIONS; }


auto
versions::get(const std::string &name) -> std::string
{
    if (VERSIONS.empty())
    {
        VERSIONS["gtkmm"]
            = std::format("{}.{}.{}", GTKMM_MAJOR_VERSION, GTKMM_MINOR_VERSION,
                          GTKMM_MICRO_VERSION);
        VERSIONS["libcurl"] = curl_version_info(CURLVERSION_NOW)->version;
        VERSIONS["jsoncpp"] = JSONCPP_VERSION_STRING;
        VERSIONS["libalpm"] = alpm_version();
        VERSIONS[APP_NAME]  = APP_VERSION;
    }

    return VERSIONS.at(name);
}


auto
versions::get() -> std::unordered_map<std::string, std::string>
{
    if (VERSIONS.empty())
    {
        VERSIONS["gtkmm"]
            = std::format("{}.{}.{}", GTKMM_MAJOR_VERSION, GTKMM_MINOR_VERSION,
                          GTKMM_MICRO_VERSION);
        VERSIONS["libcurl"] = curl_version_info(CURLVERSION_NOW)->version;
        VERSIONS["jsoncpp"] = JSONCPP_VERSION_STRING;
        VERSIONS["libalpm"] = alpm_version();
        VERSIONS[APP_NAME]  = APP_VERSION;
    }

    return VERSIONS;
}
