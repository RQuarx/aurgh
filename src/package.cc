#include <print>

#include <alpm.h>
#include <json/value.h>

#include "config.hh"
#include "package.hh"
#include "utils.hh"


namespace
{
    auto
    escape_pango_markup(const std::string &p_text) -> std::string
    {
        std::ostringstream out;
        for (char ch : p_text)
        {
            switch (ch)
            {
            case '&': out << "&amp;"; break;
            case '<': out << "&lt;"; break;
            case '>': out << "&gt;"; break;
            default:  out << ch; break;
            }
        }
        return out.str();
    }


    auto
    package_is_native(alpm_handle_t *handle, alpm_pkg_t *pkg) -> bool
    {
        const char  *pkgname { alpm_pkg_get_name(pkg) };
        alpm_list_t *sync_dbs { alpm_get_syncdbs(handle) };

        for (alpm_list_t *i { sync_dbs }; i != nullptr; i = alpm_list_next(i))
        {
            auto *pkgs { alpm_db_get_pkgcache(
                static_cast<alpm_db_t *>(i->data)) };

            std::println("{}",
                         alpm_db_get_name(static_cast<alpm_db_t *>(i->data)));

            for (alpm_list_t *j { pkgs }; j != nullptr; j = alpm_list_next(j))
            {
                std::println("  {}", alpm_pkg_get_name(
                                         static_cast<alpm_pkg_t *>(j->data)));
            }
        }


        for (alpm_list_t *i { sync_dbs }; i != nullptr; i = alpm_list_next(i))
        {
            if (alpm_db_get_pkg(static_cast<alpm_db_t *>(i->data), pkgname)
                != nullptr)
            {
                return true;
            }
        }

        return false;
    }
}


Package::Package(const Json::Value &p_pkg, bool p_from_aur)
    : from_aur(p_from_aur)
{
    valid = json_to_pkg(p_pkg);
}


auto
Package::operator[](PkgInfo p_info) -> std::string &
{
    return pkg[p_info];
}


auto
Package::get_keywords() const
    -> const std::vector<std::string> &
{
    return pkg.keywords;
}


auto
Package::is_valid() const -> bool
{
    return valid;
}


auto
Package::is_external() const -> bool
{
    return from_aur;
}


auto
Package::get_error_message() const -> std::string
{
    return error_message;
}


auto
Package::get_installed() -> std::expected<std::vector<Package>, std::string>
{
    alpm_errno_t err;
    auto *handle { alpm_initialize(ROOT.data(), DB_PATH.data(), /* NOLINT */
                                   &err) };

    if (handle == nullptr) return utils::unexpected(alpm_strerror(err));

    alpm_db_t   *localdb { alpm_get_localdb(handle) };
    alpm_list_t *pkglist { alpm_db_get_pkgcache(localdb) };

    std::vector<Package> packages;
    packages.reserve(alpm_list_count(pkglist));

    for (alpm_list_t *i { pkglist }; i != nullptr; i = alpm_list_next(i))
    {
        alpm_pkg_t *pkg { static_cast<alpm_pkg_t *>(i->data) };

        std::string name { alpm_pkg_get_name(pkg) };
        std::string version { alpm_pkg_get_version(pkg) };
        std::string desc { alpm_pkg_get_desc(pkg) };
        std::string maintainer { alpm_pkg_get_packager(pkg) };
        std::string url { alpm_pkg_get_url(pkg) };

        if (package_is_native(handle, pkg))
            name = std::format("native/{}", name);
        else
            name = std::format("foreign/{}", name);

        Json::Value package_json { Json::objectValue };

        package_json["Name"]        = name;
        package_json["Version"]     = version;
        package_json["Description"] = desc;
        package_json["Maintainer"]  = maintainer;
        package_json["URL"]         = url;

        packages.emplace_back(package_json);
    }

    alpm_release(handle);
    return packages;
}


auto
Package::json_to_pkg(const Json::Value &p_json) -> bool
{
    if (!p_json.isObject())
    {
        error_message = "Retrieved JSON is not a Json::objectValue.";
        return false;
    }

    pkg[PKG_NAME]    = escape_pango_markup(p_json["Name"].asString());
    pkg[PKG_VERSION] = escape_pango_markup(p_json["Version"].asString());

    if (p_json.isMember("Maintainer"))
        pkg[PKG_MAINTAINER]
            = escape_pango_markup(p_json["Maintainer"].asString());

    if (p_json.isMember("Description"))
        pkg[PKG_DESC]
            = escape_pango_markup(p_json["Description"].asString());

    if (p_json.isMember("NumVotes"))
        pkg[PKG_NUMVOTES]
            = escape_pango_markup(p_json["NumVotes"].asString());

    if (p_json.isMember("URL") && !p_json["URL"].isNull())
        pkg[PKG_URL] = p_json["URL"].asString();
    else
        pkg[PKG_URL] = "";

    if (p_json.isMember("Keywords"))
        for (Json::ArrayIndex i { 0 }; i < p_json["Keywords"].size(); i++)
            pkg.add_keyword(p_json["Keywords"][i].asString());

    return true;
}
