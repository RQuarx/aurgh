#pragma once
#include <filesystem>
#include <memory>

#include <alpm.h>

#include "result.hh"
#include "utils.hh"


namespace aurgh::alpm
{
    class handle
    {
        using alpm_destructor = util::destructor<alpm_handle_t, alpm_release>;

    public:
        handle(const std::filesystem::path &config = "/etc/pacman.conf");


        [[nodiscard]]
        auto get_error() const noexcept -> const char *;

    private:
        std::unique_ptr<alpm_handle_t, alpm_destructor> m_handle;

        std::filesystem::path m_root_dir = "/";
        std::filesystem::path m_db_path  = "/var/lib/pacman/";


        auto mf_init_callback(std::string_view section,
                              std::string_view key,
                              std::string_view value,
                              std::size_t      line_num) noexcept -> result<void>;


        auto mf_config_callback(std::string_view section,
                                std::string_view key,
                                std::string_view value,
                                std::size_t      line_num) noexcept -> result<void>;


        auto mf_apply_options(std::string_view key,
                              std::string_view value,
                              std::size_t      line_num) noexcept -> result<void>;

        auto mf_apply_paths(std::string_view key,
                            std::string_view value,
                            std::size_t      line_num) noexcept -> result<void>;

        auto mf_apply_path(std::string_view key,
                           std::string_view value,
                           std::size_t      line_num) noexcept -> result<void>;

        auto mf_add_repos(std::string_view key,
                          std::string_view value,
                          std::size_t      line_num) noexcept -> result<void>;
    };
}
