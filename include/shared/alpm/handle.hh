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
        auto get_repos() const noexcept -> std::span<const std::string_view>;


        [[nodiscard]]
        auto search(std::string_view name) noexcept -> result<std::vector<package>>;


        [[nodiscard]]
        auto info(std::span<const std::string> args) noexcept
            -> result<std::vector<package_details>>;

    private:
        std::unique_ptr<alpm_handle_t, alpm_destructor> m_handle;
        std::unique_ptr<config>                         m_config;

        std::vector<std::string_view> m_repos;
    };
}
