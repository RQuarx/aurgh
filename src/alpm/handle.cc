#include <sigc++/sigc++.h>

#include "alpm/handle.hh"

using aurgh::alpm::handle;
namespace fs = std::filesystem;

namespace
{
    template <typename F>
    void
    invoke_on_words(std::string_view value, F &&fn)
    {
        std::string_view trimmed { value | aurgh::views::trim };

        for (const auto &word : trimmed | std::views::split(' '))
        {
            if (std::ranges::empty(word)) continue;
            std::string_view value { word };

            fn(value);
        }
    }
}


auto
handle::create(const std::filesystem::path &config) noexcept -> result<handle>
try
{
    handle h;

    if (auto res = config::parse(config); res.has_value())
    {
        h.m_config = std::move(res.value());
        if (auto res = h.m_config.build(); res.has_value())
            h.m_handle.reset(res.value());
        else
            return res.error().unexpected();
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
