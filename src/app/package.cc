#include <json/value.h>
#include "app/package.hh"
#include "app/utils.hh"
#include "config.hh"
#include "utils.hh"

using app::Package;


namespace
{
    auto
    escape_pango_markup( const std::string &p_text ) -> std::string
    {
        std::ostringstream out;
        for (char ch : p_text) {
            switch (ch) {
            case '&': out << "&amp;"; break;
            case '<': out << "&lt;"; break;
            case '>': out << "&gt;"; break;
            default:  out << ch; break;
            }
        }
        return out.str();
    }
}


Package::Package( const std::shared_ptr<Logger> &p_logger,
                  const std::string             &p_pkg_name,
                  bool                           p_system ) :
    m_logger(p_logger)
{
    if (p_system) {

        return;
    }

    const std::string url {
        std::format("{}/info/{}", AUR_URL, p_pkg_name) };

    auto retval { perform_curl(url.c_str()) };
    if (!retval) {
        m_logger->log<ERROR>("Failed to get the info of {}: {}",
                              p_pkg_name, curl_easy_strerror(retval.error()));
        m_valid = false;
        return;
    }

    try {
        Json::Value result { Json::from_string(*retval) };
        m_valid = json_to_pkg(result["results"][Json::ArrayIndex(0)]);
    } catch (const std::exception &e) {
        m_logger->log<ERROR>("Failed to parse package {}: {}",
                              p_pkg_name, e.what());
        m_valid = false;
    }
}


Package::Package( const std::shared_ptr<Logger> &p_logger,
                  const Json::Value             &p_pkg ) :
    m_logger(p_logger)
{ m_valid = json_to_pkg(p_pkg); }


auto
Package::operator[]( PkgInfo p_info ) -> std::string &
{ return m_pkg[p_info]; }


auto
Package::get_keywords() const  -> const std::vector<std::string> &
{ return m_pkg.keywords; }


auto
Package::is_valid() const -> bool
{ return m_valid; }


auto
Package::json_to_pkg( const Json::Value &p_json ) -> bool
{
    if (!p_json.isObject()) {
        m_logger->log<ERROR>("Retrieved JSON is not an object: {}",
                              p_json.toStyledString());
        return false;
    }

    m_pkg[PKG_NAME]       = escape_pango_markup(p_json["Name"].asString());
    m_pkg[PKG_VERSION]    = escape_pango_markup(p_json["Version"].asString());
    m_pkg[PKG_MAINTAINER] = escape_pango_markup(p_json["Maintainer"].asString());
    m_pkg[PKG_DESC]       = escape_pango_markup(p_json["Description"].asString());
    m_pkg[PKG_NUMVOTES]   = escape_pango_markup(p_json["NumVotes"].asString());

    if (!p_json["URL"].isNull())
        m_pkg[PKG_URL] = p_json["URL"].asString();
    else m_pkg[PKG_URL] = "";

    for (Json::ArrayIndex i { 0 }; i < p_json["Keywords"].size(); i++)
        m_pkg.add_keyword(p_json["Keywords"][i].asString());

    return true;
}