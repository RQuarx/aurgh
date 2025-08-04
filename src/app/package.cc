#include <json/value.h>
#include "app/package.hh"
#include "app/utils.hh"
#include "config.hh"


Package::Package( const std::shared_ptr<Logger> &logger,
                  const std::string             &pkg_name,
                  bool                           system ) :
    m_logger(logger)
{
    if (system) {

        return;
    }

    const std::string url {
        std::format("{}/info/{}", AUR_URL, pkg_name) };

    auto retval { perform_curl(url.c_str()) };
    if (!retval) {
        m_logger->log<ERROR>("Failed to get the info of {}: {}",
                              pkg_name, curl_easy_strerror(retval.error()));
        m_valid = false;
        return;
    }

    Json::Value result { *json_from_string(*retval).or_else(
    [this]( const std::string &err ) -> std::expected<Json::Value, std::string>
    {
        m_logger->log<ERROR>("Malformed JSON from the AUR: {}", err);
        m_valid = false;
        return {};
    }) };

    m_valid = json_to_pkg(result["results"][Json::ArrayIndex(0)]);
}


Package::Package( const std::shared_ptr<Logger> &logger,
                      const Json::Value               &pkg ) :
    m_logger(logger)
{
    m_valid = json_to_pkg(pkg);
}


auto
Package::operator[]( PkgInfo info ) -> std::string &
{ return m_pkg[info]; }


auto
Package::get_keywords( void ) -> const std::vector<std::string> &
{ return m_pkg.keywords; }


auto
Package::is_valid( void ) -> bool
{ return m_valid; }


auto
Package::json_to_pkg( const Json::Value &json ) -> bool
{
    if (!json.isObject()) {
        m_logger->log<ERROR>("Retrieved JSON is not an object: {}",
                              json.toStyledString());
        return false;
    }

    m_pkg[PKG_NAME]       = json["Name"].asString();
    m_pkg[PKG_VERSION]    = json["Version"].asString();
    m_pkg[PKG_MAINTAINER] = json["Maintainer"].asString();
    m_pkg[PKG_DESC]       = json["Description"].asString();
    m_pkg[PKG_NUMVOTES]   = json["NumVotes"].asString();

    if (!json["URL"].isNull())
        m_pkg[PKG_URL] = json["URL"].asString();
    else m_pkg[PKG_URL] = "";

    for (Json::ArrayIndex i { 0 }; i < json["Keywords"].size(); i++) {
        m_pkg.add_keyword(json["Keywords"][i].asString());
    }

    return true;
}