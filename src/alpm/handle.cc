#include <sigc++/sigc++.h>

#include "alpm/config.hh"
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


handle::handle(const fs::path &config)
{
    
}


auto
handle::get_error() const noexcept -> const char *
{ return alpm_strerror(alpm_errno(m_handle.get())); }
