#pragma once
#include <chrono>
#include <vector>

#include <alpm.h>
#include <glibmm/ustring.h>
#include <nlohmann/json.hpp>


namespace aurgh
{
    struct package
    {
        Glib::ustring name;
        std::string   version;
        Glib::ustring description;
        std::string   repo;


        [[nodiscard]]
        static auto
        from_json(const nlohmann::json &json) -> package
        {
            return package { .name        = json["Name"].get<std::string>(),
                             .version     = json["Version"].get<std::string>(),
                             .description = json["Description"].get<std::string>(),
                             .repo        = "aur" };
        }


        [[nodiscard]]
        static auto
        from_alpm(alpm_pkg_t *pkg) -> package
        {
            return package { .name        = alpm_pkg_get_name(pkg),
                             .version     = alpm_pkg_get_version(pkg),
                             .description = alpm_pkg_get_desc(pkg),
                             .repo        = alpm_db_get_name(alpm_pkg_get_db(pkg)) };
        }
    };


    struct package_details
    {
        std::vector<std::string> licenses;
        std::vector<std::string> depends;
        std::vector<std::string> make_depends;
        std::vector<std::string> opt_depends;
        std::string              url;
        std::chrono::seconds     last_updated;


        [[nodiscard]]
        static auto
        from_json(const nlohmann::json &json) -> package_details
        {
            static auto get_str_vec
                = [](const nlohmann::json &obj, std::string_view key) -> std::vector<std::string>
            {
                if (!obj.contains(key) or obj[key].is_null()) return {};
                std::vector<std::string> out;
                out.reserve(obj[key].size());
                for (const auto &item : obj[key]) out.emplace_back(item.get<std::string>());
                return out;
            };

            return package_details {
                .licenses     = get_str_vec(json, "License"),
                .depends      = get_str_vec(json, "Depends"),
                .make_depends = get_str_vec(json, "MakeDepends"),
                .opt_depends  = get_str_vec(json, "OptDepends"),
                .url          = json.value("URL", ""),
                .last_updated = std::chrono::seconds { json["LastModified"].get<std::size_t>() },
            };
        }


        [[nodiscard]]
        static auto
        from_alpm(alpm_pkg_t *pkg) -> package_details
        {
            package_details details;

            alpm_list_t *licenses     = alpm_pkg_get_licenses(pkg);
            alpm_list_t *depends      = alpm_pkg_get_depends(pkg);
            alpm_list_t *make_depends = alpm_pkg_get_makedepends(pkg);
            alpm_list_t *opt_depends  = alpm_pkg_get_optdepends(pkg);

            details.licenses.reserve(alpm_list_count(licenses));
            details.depends.reserve(alpm_list_count(depends));
            details.make_depends.reserve(alpm_list_count(make_depends));
            details.opt_depends.reserve(alpm_list_count(opt_depends));

            for (alpm_list_t *i = licenses; i != nullptr; i = alpm_list_next(i))
                details.licenses.emplace_back(static_cast<const char *>(i->data));
            for (alpm_list_t *i = make_depends; i != nullptr; i = alpm_list_next(i))
                details.make_depends.emplace_back(static_cast<const char *>(i->data));

            for (alpm_list_t *i = depends; i != nullptr; i = alpm_list_next(i))
            {
                auto *dep = static_cast<alpm_depend_t *>(i->data);

                char *str = alpm_dep_compute_string(dep);
                if (str == nullptr) continue;

                details.depends.emplace_back(str);
                free(str);
            }

            for (alpm_list_t *i = opt_depends; i != nullptr; i = alpm_list_next(i))
            {
                auto *dep = static_cast<alpm_depend_t *>(i->data);

                std::string entry { dep->name != nullptr ? dep->name : "" };

                if (dep->desc != nullptr and *dep->desc != '\0')
                {
                    entry += ": ";
                    entry += dep->desc;
                }

                details.opt_depends.emplace_back(std::move(entry));
            }

            details.url          = alpm_pkg_get_url(pkg) != nullptr ? alpm_pkg_get_url(pkg) : "";
            details.last_updated = std::chrono::seconds { alpm_pkg_get_builddate(pkg) };
            return details;
        }
    };
}
