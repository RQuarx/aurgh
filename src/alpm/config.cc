#include <glob.h>
#include <sigc++/sigc++.h>
#include <sys/utsname.h>

#include "alpm/config.hh"

using config = aurgh::alpm::config;

namespace
{
    [[nodiscard]]
    auto
    glob_error_to_string(int err) noexcept -> std::string_view
    {
        switch (err)
        {
        case GLOB_NOSPACE: return "no space available to perform globbing";
        case GLOB_ABORTED: return "read error occurred while globbing";
        case GLOB_NOMATCH: return "no match found";
        default:           return {};
        }
    }


    [[nodiscard]]
    auto
    globdir(const std::string &pattern) noexcept
        -> aurgh::result<std::vector<std::filesystem::path>>
    try
    {
        glob_t buff;
        if (int res = glob(pattern.c_str(), GLOB_NOCHECK, nullptr, &buff); res != 0)
            return aurgh::error { "{}", glob_error_to_string(res) }.unexpected();

        std::vector<std::filesystem::path> paths;
        paths.reserve(buff.gl_pathc);

        for (std::size_t i = 0; i < buff.gl_pathc; i++) paths.emplace_back(buff.gl_pathv[i]);

        globfree(&buff);
        return paths;
    }
    catch (const std::exception &e)
    {
        return aurgh::error { "failed to perform globbing: {}", e.what() }.unexpected();
    }


    [[nodiscard]]
    auto
    split_on_space_to_string(std::string_view string) -> std::vector<std::string>
    {
        return string | std::views::split(' ')
             | std::views::filter([](auto &&it) { return !std::ranges::empty(it); })
             | std::ranges::to<std::vector<std::string>>();
    }


    [[nodiscard]]
    auto
    split_on_space_to_path(std::string_view string) -> std::vector<std::filesystem::path>
    {
        return string | std::views::split(' ')
             | std::views::filter([](auto &&it) { return !std::ranges::empty(it); })
             | std::views::transform([](auto &&it) { return std::string_view { it }; })
             | std::views::transform([](std::string_view s)
                                     { return std::filesystem::canonical(s); })
             | std::ranges::to<std::vector<std::filesystem::path>>();
    }


    struct siglevel_rule
    {
        std::string_view name;

        int pkg_set;
        int pkg_unset;

        int db_set;
        int db_unset;
    };


    constexpr std::array<siglevel_rule, 5> siglevel_rules {
        { {
              .name      = "Never",
              .pkg_unset = ALPM_SIG_PACKAGE,
              .db_unset  = ALPM_SIG_DATABASE,
          }, /* NOLINT */
          {

              .name    = "Optional",
              .pkg_set = ALPM_SIG_PACKAGE | ALPM_SIG_PACKAGE_OPTIONAL,
              .db_set  = ALPM_SIG_DATABASE | ALPM_SIG_DATABASE_OPTIONAL,
          }, /* NOLINT */
          {
              .name      = "Required",
              .pkg_set   = ALPM_SIG_PACKAGE,
              .pkg_unset = ALPM_SIG_PACKAGE_OPTIONAL,
              .db_set    = ALPM_SIG_DATABASE,
              .db_unset  = ALPM_SIG_DATABASE_OPTIONAL,
          }, {
              .name      = "TrustedOnly",
              .pkg_unset = ALPM_SIG_PACKAGE_MARGINAL_OK | ALPM_SIG_PACKAGE_UNKNOWN_OK,
              .db_unset  = ALPM_SIG_DATABASE_MARGINAL_OK | ALPM_SIG_DATABASE_UNKNOWN_OK,
          }, /* NOLINT */
          {
              .name    = "TrustAll",
              .pkg_set = ALPM_SIG_PACKAGE_MARGINAL_OK | ALPM_SIG_PACKAGE_UNKNOWN_OK,
              .db_set  = ALPM_SIG_DATABASE_MARGINAL_OK | ALPM_SIG_DATABASE_UNKNOWN_OK,
          } }  /* NOLINT */
    };


    [[nodiscard]]
    auto
    process_siglevel(auto &&values, int *storage, int *storage_mask) noexcept -> aurgh::result<void>
    {
        using namespace std::string_view_literals;

        int level = *storage;
        int mask  = *storage_mask;

        std::string bad_values;

        for (const auto &original : values)
        {
            std::string_view value { original | aurgh::views::trim };

            bool package  = false;
            bool database = false;

            if (value.starts_with("Package"))
            {
                value.remove_prefix("Package"sv.length());
                package = true;
            }
            else if (value.starts_with("Database"))
            {
                value.remove_prefix("Database"sv.length());
                database = true;
            }
            else
                package = database = true;

            const auto *it = std::ranges::find(siglevel_rules, value, &siglevel_rule::name);
            if (it == siglevel_rules.end())
            {
                if (!bad_values.empty()) bad_values += ", ";
                bad_values += original;
                continue;
            }

            if (package)
            {
                level = (level & ~it->pkg_unset) | it->pkg_set;
                mask |= it->pkg_set | it->pkg_unset;
            }
            if (database)
            {
                level = (level & ~it->db_unset) | it->db_set;
                mask |= it->db_set | it->db_unset;
            }

            level &= ~ALPM_SIG_USE_DEFAULT;
        }

        if (!bad_values.empty())
            return aurgh::error { "invalid siglevel values: \"{}\"", bad_values }.unexpected();

        *storage      = level;
        *storage_mask = mask;
        return {};
    }


    [[nodiscard]]
    auto
    process_usage(auto &&values) noexcept -> aurgh::result<int>
    {
        int usage = 0;

        for (const auto &key : values)
        {
            if (key == "Sync")
                usage |= ALPM_DB_USAGE_SYNC;
            else if (key == "Search")
                usage |= ALPM_DB_USAGE_SEARCH;
            else if (key == "Install")
                usage |= ALPM_DB_USAGE_INSTALL;
            else if (key == "Upgrade")
                usage |= ALPM_DB_USAGE_UPGRADE;
            else if (key == "All")
                usage |= ALPM_DB_USAGE_ALL;
            else
                return aurgh::error { "unrecognized option \"{}\"", key }.unexpected();
        }

        return usage;
    }


    auto
    replace_server_vars(const aurgh::alpm::config &cfg,
                        aurgh::alpm::repo         &repo,
                        auto                     &&servers) noexcept -> aurgh::result<void>
    {
        auto replace = [&](std::string &url) -> aurgh::result<void>
        {
            if (cfg.architecture.empty() and url.contains("$arch"))
                return aurgh::error {
                    R"(mirror "{}" contains "$arch" variable, but no "Architecture" is defined.)",
                    url
                }
                    .unexpected();

            auto replace_all = [&](std::string_view token, std::string_view value)
            {
                for (std::size_t i = url.find(token); i != std::string::npos;
                     i             = url.find(token, i + value.size()))
                    url.replace(i, token.size(), value);
            };

            if (!cfg.architecture.empty()) replace_all("$arch", cfg.architecture.front());
            replace_all("$repo", repo.name);

            return {};
        };

        for (auto &url : repo.*servers)
            if (auto res = replace(url); !res) return res.error().unexpected();
        return {};
    }


    auto
    register_repo(alpm_handle_t *handle, const aurgh::alpm::repo &repo) noexcept
        -> aurgh::result<void>
    {
        alpm_db_t *db = alpm_register_syncdb(handle, repo.name.c_str(), repo.siglevel);

        if (db == nullptr)
            return aurgh::error { "failed to register database \"{}\": {}", repo.name,
                                  alpm_strerror(alpm_errno(handle)) }
                .unexpected();

        alpm_db_set_usage(db, repo.usage);

        for (const auto &url : repo.cache_servers)
            if (alpm_db_add_cache_server(db, url.c_str()) != 0)
                return aurgh::error { "failed to add cache server URL to database \"{}\": {}",
                                      repo.name, alpm_strerror(alpm_errno(handle)) }
                    .unexpected();

        for (const auto &url : repo.servers)
            if (alpm_db_add_server(db, url.c_str()) != 0)
                return aurgh::error { "failed to add server URL to database \"{}\": {}", repo.name,
                                      alpm_strerror(alpm_errno(handle)) }
                    .unexpected();
        return {};
    }


    auto
    get_system_arch() -> std::string
    {
        struct utsname un;
        uname(&un);
        return un.machine;
    }
}


auto
config::parse(const std::filesystem::path &pacman_conf) noexcept -> result<std::unique_ptr<config>>
try
{
    auto cfg = std::make_unique<config>();

    if (auto res
        = ini::parse(pacman_conf, [&](ini::callback_data data) { return cfg->mf_parse_cb(data); });
        !res)
        return res.error().unexpected();

    if (auto res = cfg->mf_set_defaults(); !res) return res.error().unexpected();

    return cfg;
}
catch (const std::exception &e)
{
    return error { "failed to parse config file \"{}\": {}", pacman_conf.c_str(), e.what() }
        .unexpected();
}


auto
config::mf_parse_cb(ini::callback_data data, int depth) noexcept -> result<void>
{
    if (data.key.empty() and data.value.empty() and data.section != "options")
        repos.emplace_back(std::string { data.section });

    if (data.key == "Include") return mf_process_include(data, depth);
    if (data.section.empty())
        return error { "parsing error at {}:{}: all directives must belong to a section",
                       data.filepath, data.line_num }
            .unexpected();

    if (data.section == "options") return mf_parse_options(data);
    return mf_parse_repo(data);
}


auto
config::mf_process_include(ini::callback_data data, int depth) noexcept -> result<void>
try
{
    if (data.value.empty())
        return error { "parsing error at {}:{}: include directive requires a value", data.filepath,
                       data.line_num }
            .unexpected();

    if (depth >= config::max_include_depth)
        return error { "parsing error: exceeded maximum recursion depth of {}",
                       config::max_include_depth }
            .unexpected();

    std::vector<std::filesystem::path> paths;

    if (auto res = globdir(std::string { data.value }); res.has_value())
        paths = std::move(res.value());
    else
        return error { "include error at {}:{}: {}", data.filepath, data.line_num,
                       res.error().message() }
            .unexpected();

    for (auto &path : paths)
        if (auto res = ini::parse(
                path, [&](ini::callback_data data) { return mf_parse_cb(data, depth + 1); },
                std::string { data.section });
            !res)
            return res.error().unexpected();

    return {};
}
catch (const std::exception &e)
{
    return error { "failed to process include: {}", e.what() }.unexpected();
}


auto
config::mf_parse_options(ini::callback_data data) noexcept -> result<void>
try
{
    if (data.value.empty())
    {
        use_syslog                 = data.key == "UseSyslog";
        check_space                = data.key == "CheckSpace";
        disable_download_timeout   = data.key == "DisableDownloadTimeout";
        disable_sandbox            = data.key == "DisableSandbox";
        disable_sandbox_filesystem = data.key == "DisableSandboxFilesystem";
        disable_sandbox_syscalls   = data.key == "DisableSandboxSyscalls";

        return {};
    }

    if (auto res = mf_parse_siglevels(data); !res) return res.error().unexpected();

    if (data.key == "HoldPkg") hold_pkg = split_on_space_to_string(data.value);
    if (data.key == "IgnorePkg") ignore_pkg = split_on_space_to_string(data.value);
    if (data.key == "IgnoreGroup") ignore_group = split_on_space_to_string(data.value);
    if (data.key == "NoExtract") no_extract = split_on_space_to_string(data.value);
    if (data.key == "Architecture")
    {
        architecture = split_on_space_to_string(data.value);

        if (auto it = std::ranges::find(architecture, "auto"); it != architecture.end())
            *it = get_system_arch();
    }

    if (data.key == "CacheDir") cache_dir = split_on_space_to_path(data.value);
    if (data.key == "HookDir") hook_dir = split_on_space_to_path(data.value);

    if (data.key == "ParallelDownloads")
    {
        if (auto res = util::to_integral<unsigned int>(data.value.begin(), data.value.end());
            res.has_value())
            parallel_downloads = res.value();
        else
        {
            std::string_view msg;

            switch (res.error())
            {
                using enum std::errc;
            case invalid_argument:    msg = "value is not a valid integer"; break;
            case result_out_of_range: msg = "value is out-of-range "; break;
            default:                  msg = "failed to convert string to integer"; break;
            }

            return error { "parsing error at {}:{}: {}", data.filepath, data.line_num, msg }
                .unexpected();
        }
    }

    if (data.key == "DownloadUser") sandbox_user = data.value;

    namespace fs = std::filesystem;

    if (data.key == "RootDir") root_dir = fs::canonical(data.value);
    if (data.key == "DBPath") db_path = fs::canonical(data.value);
    if (data.key == "GPGDir") gpg_dir = fs::canonical(data.value);
    if (data.key == "LogFile") log_file = fs::canonical(data.value);

    return {};
}
catch (const std::exception &e)
{
    return error { "parsing error at {}:{}: {}", data.filepath, data.line_num, e.what() }
        .unexpected();
}


auto
config::mf_parse_siglevels(ini::callback_data data) noexcept -> result<void>
{
    auto get_siglevel = [&data](int *siglevel, int *mask) noexcept -> result<void>
    {
        auto res = process_siglevel(split_on_space_to_string(data.value), siglevel, mask);
        return !res ? error { "parsing error at {}:{}: {}", data.filepath, data.line_num,
                              res.error().message() }
                          .unexpected()
                    : result<void> {};
    };

    if (data.key == "SigLevel")
        if (auto res = get_siglevel(&siglevel, &siglevel_mask); !res)
            return res.error().unexpected();
    if (data.key == "LocalFileSigLevel")
        if (auto res = get_siglevel(&local_siglevel, &local_siglevel_mask); !res)
            return res.error().unexpected();
    if (data.key == "RemoteFileSigLevel")
        if (auto res = get_siglevel(&remote_siglevel, &remote_siglevel_mask); !res)
            return res.error().unexpected();
    return {};
}


auto
config::mf_parse_repo(ini::callback_data data) noexcept -> result<void>
try
{
    repo *repo = nullptr;

    if (auto it = std::ranges::find(repos, data.section, &repo::name); it != repos.end())
        repo = &*it;
    else
        return {};

    auto err = [&data](std::string_view name)
    {
        return error { "parsing error at {}:{}: directive \"{}\" needs a value", data.filepath,
                       data.line_num, name }
            .unexpected();
    };

    if (data.key == "CacheServer")
    {
        if (data.value.empty()) return err("CacheServer");
        repo->cache_servers.emplace_back(data.value);
    }

    if (data.key == "Server")
    {
        if (data.value.empty()) return err("Server");
        repo->servers.emplace_back(data.value);
    }

    if (data.key == "SigLevel")
    {
        if (data.value.empty()) return err("SigLevel");
        auto res = process_siglevel(split_on_space_to_string(data.value), &repo->siglevel,
                                    &repo->siglevel_mask);
        if (!res)
            return error { "parsing error at {}:{}: {}", data.filepath, data.line_num,
                           res.error().message() }
                .unexpected();
    }

    if (data.key == "Usage")
    {
        if (data.value.empty()) return err("Usage");
        auto res = process_usage(split_on_space_to_string(data.value));
        if (!res)
            return error { "parsing error at {}:{}: {}", data.filepath, data.line_num,
                           res.error().message() }
                .unexpected();
        repo->usage = res.value();
    }

    return {};
}
catch (const std::exception &e)
{
    return error { "parse error at {}:{}: failed to parse repo ({})", data.filepath, data.line_num,
                   e.what() }
        .unexpected();
}


auto
config::mf_set_defaults() noexcept -> result<void>
{
    if (!root_dir.empty())
    {
        if (db_path.empty()) db_path = root_dir / default_db_path;
        if (log_file.empty()) log_file = root_dir / default_log_file;
    }
    else
    {
        root_dir = default_root_dir;
        if (db_path.empty()) db_path = default_db_path;
        if (log_file.empty()) log_file = default_log_file;
    }

    if (gpg_dir.empty()) gpg_dir = default_gpg_dir;

    if (cache_dir.empty()) cache_dir.emplace_back(default_cache_dir);
    if (hook_dir.empty()) hook_dir.emplace_back(default_hook_dir);

    auto merge_siglevel
        = [](int base, int over, int mask) { return mask ? (over & mask) | (base & ~mask) : over; };

    local_siglevel  = merge_siglevel(siglevel, local_siglevel, local_siglevel_mask);
    remote_siglevel = merge_siglevel(siglevel, remote_siglevel, remote_siglevel_mask);

    for (auto &repo : repos)
    {
        if (repo.usage == 0) repo.usage = ALPM_DB_USAGE_ALL;
        repo.siglevel = merge_siglevel(siglevel, repo.siglevel, repo.siglevel_mask);

        if (auto res = replace_server_vars(*this, repo, &repo::servers); !res)
            return res.error().unexpected();
        if (auto res = replace_server_vars(*this, repo, &repo::cache_servers); !res)
            return res.error().unexpected();
    }

    return {};
}


auto
config::build() noexcept -> result<alpm_handle_t *>
{
    alpm_errno_t   err;
    alpm_handle_t *handle;

    handle = alpm_initialize(root_dir.c_str(), db_path.c_str(), &err);
    if (handle == nullptr)
    {
        return error { "failed to initialize libalpm (root: {}, dbpath: {}): {}", root_dir.c_str(),
                       db_path.c_str(), alpm_strerror(err) }
            .unexpected();
    }

    alpm_option_set_logcb(handle, config::log_callback, this);
    alpm_option_set_dlcb(handle, config::download_callback, this);
    alpm_option_set_eventcb(handle, config::event_callback, this);
    alpm_option_set_questioncb(handle, config::question_callback, this);
    alpm_option_set_progresscb(handle, config::progress_callback, this);

    auto get_error = [&] { return alpm_strerror(alpm_errno(handle)); };

    if (alpm_option_set_logfile(handle, log_file.c_str()) != 0)
        return error { "failed to set log file to \"{}\": {}", log_file.c_str(), get_error() }
            .unexpected();

    if (alpm_option_set_gpgdir(handle, gpg_dir.c_str()) != 0)
        return error { "failed to set GnuPG dir to \"{}\": {}", gpg_dir.c_str(), get_error() }
            .unexpected();

    for (const auto &dir : hook_dir)
        if (alpm_option_add_hookdir(handle, dir.c_str()) != 0)
            return error { "failed to add hook dir \"{}\": {}", dir.c_str(), get_error() }
                .unexpected();

    for (const auto &dir : cache_dir)
        if (alpm_option_add_cachedir(handle, dir.c_str()) != 0)
            return error { "failed to add cache dir \"{}\": {}", dir.c_str(), get_error() }
                .unexpected();

    alpm_option_set_default_siglevel(handle, siglevel);
    alpm_option_set_local_file_siglevel(handle, local_siglevel);
    alpm_option_set_remote_file_siglevel(handle, remote_siglevel);

    for (const auto &repo : repos)
        if (auto res = register_repo(handle, repo); !res) return res.error().unexpected();

    for (const auto &arch : architecture)
        if (alpm_option_add_architecture(handle, arch.c_str()) != 0)
            return error { "failed to add architecture \"{}\": {}", arch, get_error() }
                .unexpected();

    alpm_option_set_checkspace(handle, check_space);
    alpm_option_set_usesyslog(handle, use_syslog);

    if (alpm_option_set_sandboxuser(handle, sandbox_user.c_str()) != 0)
        return error { "failed to set DownloadUser \"{}\" (user doesn't exist)", sandbox_user }
            .unexpected();

    alpm_option_set_disable_sandbox_filesystem(handle, disable_sandbox_filesystem);
    alpm_option_set_disable_sandbox_syscalls(handle, disable_sandbox_syscalls);

    for (const auto &pkg : ignore_pkg) alpm_option_add_ignorepkg(handle, pkg.c_str());
    for (const auto &group : ignore_group) alpm_option_add_ignoregroup(handle, group.c_str());
    for (const auto &pkg : no_upgrade) alpm_option_add_noupgrade(handle, pkg.c_str());
    for (const auto &pkg : no_extract) alpm_option_add_noextract(handle, pkg.c_str());

    alpm_option_set_disable_dl_timeout(handle, disable_download_timeout);
    alpm_option_set_parallel_downloads(handle, parallel_downloads);

    return handle;
}


void
config::log_callback(void *ctx, alpm_loglevel_t level, const char *fmt, va_list args)
{
    auto *cfg = static_cast<config *>(ctx);

    if (fmt == nullptr || std::string_view { fmt }.length() == 0) return;

    va_list args_copy;
    va_copy(args_copy, args);
    std::size_t needed = std::vsnprintf(nullptr, 0, fmt, args_copy);
    va_end(args_copy);

    std::string message;
    message.resize_and_overwrite(needed, [&](char *buf, std::size_t n) -> std::size_t
                                 { return std::vsnprintf(buf, n + 1, fmt, args); });

    cfg->signal_on_log.emit(level, message);
}


void
config::download_callback(void                      *ctx,
                          const char                *filename,
                          alpm_download_event_type_t event,
                          void                      *data)
{
    auto *cfg = static_cast<config *>(ctx);
    switch (event)
    {
    case ALPM_DOWNLOAD_INIT:
        cfg->signal_on_download.emit(filename, static_cast<alpm_download_event_init_t *>(data));
        break;
    case ALPM_DOWNLOAD_PROGRESS:
        cfg->signal_on_download.emit(filename,
                                     static_cast<_alpm_download_event_progress_t *>(data));
        break;
    case ALPM_DOWNLOAD_RETRY:
        cfg->signal_on_download.emit(filename, static_cast<alpm_download_event_retry_t *>(data));
        break;
    case ALPM_DOWNLOAD_COMPLETED:
        cfg->signal_on_download.emit(filename,
                                     static_cast<alpm_download_event_completed_t *>(data));
        break;
    }
}


void
config::event_callback(void *ctx, alpm_event_t *event)
{ static_cast<config *>(ctx)->signal_on_event.emit(event); }


void
config::question_callback(void *ctx, alpm_question_t *question)
{ static_cast<config *>(ctx)->signal_on_question.emit(question); }


void
config::progress_callback(void           *ctx,
                          alpm_progress_t progress,
                          const char     *pkg,
                          int             percent,
                          std::size_t     total_amount,
                          std::size_t     current_amount)
{
    static_cast<config *>(ctx)->signal_on_progress.emit(progress, pkg, percent, total_amount,
                                                        current_amount);
}
