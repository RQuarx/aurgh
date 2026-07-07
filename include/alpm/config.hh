#pragma once
#include <filesystem>
#include <vector>

#include <alpm.h>
#include <sigc++/signal.h>

#include "ini.hh"
#include "result.hh"


namespace aurgh::alpm
{
    struct repo
    {
        std::string name;

        std::vector<std::string> cache_servers;
        std::vector<std::string> servers;

        int usage         = 0;
        int siglevel      = ALPM_SIG_USE_DEFAULT;
        int siglevel_mask = 0;


        repo(std::string name) : name { std::move(name) } {}
    };


    struct config
    {
        static constexpr std::uint8_t     max_include_depth = 10;
        static constexpr std::string_view default_root_dir  = "/";
        static constexpr std::string_view default_db_path   = "/var/lib/pacman";
        static constexpr std::string_view default_gpg_dir   = "/etc/pacman.d/gnupg/";
        static constexpr std::string_view default_log_file  = "/var/log/pacman.log";
        static constexpr std::string_view default_cache_dir = "/var/cache/pacman/pkg/";
        static constexpr std::string_view default_hook_dir  = "/usr/share/libalpm/hooks";

        std::filesystem::path root_dir = "/";
        std::filesystem::path db_path  = "/var/lib/pacman/";
        std::filesystem::path gpg_dir  = "/etc/pacman.d/gnupg/";
        std::filesystem::path log_file = "/var/log/pacman.log";

        std::vector<std::filesystem::path> cache_dir { "/var/cache/pacman/pkg/" };
        std::vector<std::filesystem::path> hook_dir { "/usr/share/libalpm/hooks" };

        std::vector<std::string> hold_pkg;
        std::vector<std::string> ignore_pkg;
        std::vector<std::string> ignore_group;
        std::vector<std::string> architecture;
        std::vector<std::string> no_upgrade;
        std::vector<std::string> no_extract;

        std::string  sandbox_user;
        unsigned int parallel_downloads;

        /* bitfield for alpm_transflag_t */
        int flag = 0;

        int siglevel        = ALPM_SIG_PACKAGE | ALPM_SIG_DATABASE;
        int local_siglevel  = ALPM_SIG_USE_DEFAULT;
        int remote_siglevel = ALPM_SIG_USE_DEFAULT;

        int siglevel_mask        = 0;
        int local_siglevel_mask  = 0;
        int remote_siglevel_mask = 0;

        bool use_syslog                 = false;
        bool check_space                = false;
        bool disable_download_timeout   = false;
        bool disable_sandbox            = false;
        bool disable_sandbox_filesystem = false;
        bool disable_sandbox_syscalls   = false;

        std::vector<repo> repos;

        using download_data = std::variant<alpm_download_event_init_t *,
                                           alpm_download_event_progress_t *,
                                           alpm_download_event_retry_t *,
                                           alpm_download_event_completed_t *>;

        sigc::signal<void(alpm_loglevel_t, std::string)>    signal_on_log;
        sigc::signal<void(std::string_view, download_data)> signal_on_download;
        sigc::signal<void(alpm_event_t *)>                  signal_on_event;
        sigc::signal<void(alpm_question_t *)>               signal_on_question;
        sigc::signal<void(alpm_progress_t, std::string_view, int, std::size_t, std::size_t)>
            signal_on_progress;


        [[nodiscard]]
        static auto parse(const std::filesystem::path &pacman_conf) noexcept -> result<config>;


        [[nodiscard]]
        auto build() noexcept -> result<alpm_handle_t *>;

    private:
        auto mf_parse_cb(ini::callback_data data, int depth = 0) noexcept -> result<void>;
        auto mf_process_include(ini::callback_data data, int depth) noexcept -> result<void>;
        auto mf_parse_options(ini::callback_data data) noexcept -> result<void>;
        auto mf_parse_siglevels(ini::callback_data data) noexcept -> result<void>;
        auto mf_parse_repo(ini::callback_data data) noexcept -> result<void>;

        auto mf_set_defaults() noexcept -> result<void>;


        static void log_callback(void *ctx, alpm_loglevel_t level, const char *fmt, va_list args);
        static void download_callback(void                      *ctx,
                                      const char                *filename,
                                      alpm_download_event_type_t event,
                                      void                      *data);
        static void event_callback(void *ctx, alpm_event_t *event);
        static void question_callback(void *ctx, alpm_question_t *question);
        static void progress_callback(void           *ctx,
                                      alpm_progress_t progress,
                                      const char     *pkg,
                                      int             percent,
                                      std::size_t     total_amount,
                                      std::size_t     current_amount);
    };
}
