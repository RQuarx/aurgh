#include <sigc++/sigc++.h>

#include "alpm/handle.hh"

using aurgh::alpm::handle;
namespace fs = std::filesystem;


auto
handle::create(const std::filesystem::path &config) noexcept -> result<handle>
try
{
    handle h;

    if (auto res = config::parse(config); res.has_value())
    {
        h.m_config = std::move(res.value());

        if (auto res = h.m_config->build(); res.has_value())
            h.m_handle.reset(res.value());
        else
            return res.error().unexpected();

        h.m_repos = h.m_config->repos | std::views::transform(&repo::name)
                  | std::ranges::to<std::vector<std::string_view>>();
    }
    else
        return res.error().unexpected();

    return std::move(h);
}
catch (const std::exception &e)
{
    return error { "failed to create libalpm handle: {}", e.what() }.unexpected();
}


auto
handle::get_error() const noexcept -> const char *
{ return alpm_strerror(alpm_errno(m_handle.get())); }


auto
handle::search(std::string_view name) noexcept -> result<std::vector<package>>
{
    std::string search { name };

    alpm_list_t *syncdbs = alpm_get_syncdbs(m_handle.get());
    alpm_list_t *needle  = nullptr;

    std::vector<package> packages;

    needle = alpm_list_add(needle, static_cast<void *>(search.data()));

    for (alpm_list_t *i = syncdbs; i != nullptr; i = alpm_list_next(i))
    {
        alpm_list_t *res = nullptr;
        auto        *db  = static_cast<alpm_db_t *>(i->data);

        if (alpm_db_search(db, needle, &res) != 0)
            return error { "failed to search for a package on syncdb \"{}\": {}",
                           alpm_db_get_name(db), get_error() }
                .unexpected();

        packages.reserve(alpm_list_count(res));

        for (alpm_list_t *i = res; i != nullptr; i = alpm_list_next(i))
        {
            auto *pkg = static_cast<alpm_pkg_t *>(i->data);
            if (pkg != nullptr) packages.emplace_back(package::from_alpm(pkg));
        }

        alpm_list_free(res);
    }

    alpm_list_free(needle);
    return packages;
}


auto
handle::info(std::span<const std::string> args) noexcept -> result<std::vector<package_details>>
{
    alpm_list_t *syncdbs = alpm_get_syncdbs(m_handle.get());

    std::vector<package_details> details;

    for (const auto &name : args)
        for (alpm_list_t *i = syncdbs; i != nullptr; i = alpm_list_next(i))
        {
            auto *db = static_cast<alpm_db_t *>(i->data);
            if (db == nullptr) continue;

            if (auto *pkg = alpm_db_get_pkg(db, name.c_str()); pkg != nullptr)
                details.emplace_back(package_details::from_alpm(pkg));
        }

    return details;
}


auto
handle::get_repos() const noexcept -> std::span<const std::string_view>
{ return m_repos; }
