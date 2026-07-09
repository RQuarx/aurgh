#pragma once
#include <filesystem>
#include <memory>

#include <alpm.h>

#include "alpm/config.hh"
#include "package.hh"
#include "result.hh"
#include "utils.hh"


namespace aurgh::alpm
{
    class handle
    {
        using alpm_destructor = util::destructor<alpm_handle_t, alpm_release>;

    public:
        [[nodiscard]]
        static auto create(const std::filesystem::path &config = "/etc/pacman.conf") noexcept
            -> result<handle>;


        [[nodiscard]]
        auto get_error() const noexcept -> const char *;


        [[nodiscard]]
        auto search(std::string_view name) noexcept -> result<std::vector<package>>;

    private:
        std::unique_ptr<alpm_handle_t, alpm_destructor> m_handle;
        config                                          m_config;
    };
}
