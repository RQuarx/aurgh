#include <alpm.h>
#include <json/value.h>

#include "config.hh"
#include "logger.hh"
#include "package.hh"
#include "utils.hh"


namespace
{
    enum class PackageLocality : std::uint8_t
    {
        NATIVE,
        FOREIGN,
        ERROR
    };


    auto
    escape_pango_markup(const std::string &text) -> std::string
    {
        std::ostringstream out;
        for (char ch : text)
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
    get_package_locality(alpm_handle_t *handle, alpm_pkg_t *pkg)
        -> PackageLocality
    {
        const char  *pkgname { alpm_pkg_get_name(pkg) };
        alpm_list_t *sync_dbs { alpm_get_syncdbs(handle) };

        if (sync_dbs == nullptr)
            return PackageLocality::ERROR;

        for (alpm_list_t *j { sync_dbs }; j != nullptr; j = alpm_list_next(j))
            if (alpm_db_get_pkg(static_cast<alpm_db_t *>(j->data), pkgname)
                != nullptr)
                return PackageLocality::NATIVE;

        return PackageLocality::FOREIGN;
    }
}


PackageEntry::PackageEntry(const Json::Value &pkg, bool from_aur)
    : m_is_from_aur(from_aur)
{
    m_is_valid = json_to_pkg(pkg);
}


auto
PackageEntry::operator[](PackageField field) -> std::string &
{
    return m_data[field];
}


auto
PackageEntry::get_keywords() const -> const std::vector<std::string> &
{
    return m_data.keywords;
}


auto
PackageEntry::is_valid() const -> bool
{
    return m_is_valid;
}


auto
PackageEntry::is_external() const -> bool
{
    return m_is_from_aur;
}


auto
PackageEntry::get_error_message() const -> std::string
{
    return m_error_message;
}


auto
PackageEntry::get_installed()
    -> std::expected<std::vector<PackageEntry>, std::string>
{
    alpm_errno_t err;
    auto *handle { alpm_initialize(ROOT.data(), DB_PATH.data(), /* NOLINT */
                                   &err) };

    if (handle == nullptr) return utils::unexpected(alpm_strerror(err));

    alpm_db_t   *localdb { alpm_get_localdb(handle) };
    alpm_list_t *pkglist { alpm_db_get_pkgcache(localdb) };

    std::vector<PackageEntry> packages;
    packages.reserve(alpm_list_count(pkglist));

    for (alpm_list_t *i { pkglist }; i != nullptr; i = alpm_list_next(i))
    {
        alpm_pkg_t *pkg { static_cast<alpm_pkg_t *>(i->data) };

        std::string name { alpm_pkg_get_name(pkg) };
        std::string version { alpm_pkg_get_version(pkg) };
        std::string desc { alpm_pkg_get_desc(pkg) };
        std::string maintainer { alpm_pkg_get_packager(pkg) };
        std::string url { alpm_pkg_get_url(pkg) };

        switch (get_package_locality(handle, pkg))
        {
        case PackageLocality::NATIVE:
            name = std::format("native/{}", name);break;
        case PackageLocality::FOREIGN:
            name = std::format("foreign/{}", name); break;
        case PackageLocality::ERROR:
            return utils::unexpected(alpm_strerror(alpm_errno(handle)));
        }

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
PackageEntry::json_to_pkg(const Json::Value &json) -> bool
{
    if (!json.isObject())
    {
        m_error_message = "Retrieved JSON is not a Json::objectValue";
        return false;
    }

    using enum PackageField;

    m_data[NAME]    = escape_pango_markup(json["Name"].asString());
    m_data[VERSION] = escape_pango_markup(json["Version"].asString());

    if (json.isMember("Maintainer"))
        m_data[MAINTAINER] = escape_pango_markup(json["Maintainer"].asString());

    if (json.isMember("Description"))
        m_data[DESC] = escape_pango_markup(json["Description"].asString());

    if (json.isMember("NumVotes"))
        m_data[NUMVOTES] = escape_pango_markup(json["NumVotes"].asString());

    if (json.isMember("URL") && !json["URL"].isNull())
        m_data[URL] = json["URL"].asString();
    else
        m_data[URL] = "";

    if (json.isMember("Keywords"))
        for (Json::ArrayIndex i { 0 }; i < json["Keywords"].size(); i++)
            m_data.add_keyword(json["Keywords"][i].asString());

    return true;
}
