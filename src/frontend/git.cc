#include <utility>

#include <git2.h>

#include "git.hh"

using aurgh::git::cloning;

namespace
{
    [[nodiscard]]
    auto
    get_libgit2_error() noexcept -> std::string_view
    {
        const auto *err = git_error_last();
        if (err != nullptr and err->message != nullptr) return err->message;
        return { "no error specified" };
    }


    [[nodiscard]]
    auto
    get_name(std::string_view url) -> std::string
    {
        while (!url.empty() and url.back() == '/') url.remove_suffix(1);

        std::size_t pos = url.find_last_of("/:");
        std::string name { pos == std::string_view::npos ? url : url.substr(pos + 1) };

        if (name.ends_with(".git")) name.resize(name.size() - 4);

        return name;
    }
}


cloning::cloning(std::string_view url, std::filesystem::path base)
    : m_url { url }, m_base { std::move(base) }, m_dst { m_base / get_name(m_url) }
{
    if (!std::filesystem::exists(m_base)) std::filesystem::create_directories(m_base);
    if (!std::filesystem::is_directory(m_base))
        throw error { "clone base directory is not a directory" };

    m_dispatch_progress.connect(sigc::mem_fun(*this, &cloning::mf_on_progress));
    m_dispatch_done.connect(sigc::mem_fun(*this, &cloning::mf_on_done));

    m_thread = std::jthread { [this](std::stop_token token) { mf_run(std::move(token)); } };
}


void
cloning::cancel()
{ m_thread.request_stop(); }


void
cloning::mf_run(std::stop_token token)
{
    m_stop_token = std::move(token);

    if (git_libgit2_init() < 0)
    {
        m_pending_error = error { "failed to initialize libgit2: {}", get_libgit2_error() };
        m_dispatch_done.emit();
    }

    git_clone_options opts;
    git_clone_options_init(&opts, GIT_CLONE_OPTIONS_VERSION);

    opts.fetch_opts.callbacks.transfer_progress = &cloning::transfer_progress_callback;
    opts.fetch_opts.callbacks.payload           = this;

    git_repository *repo = nullptr;

    if (std::filesystem::exists(m_dst)) std::filesystem::remove_all(m_dst);

    int res = git_clone(&repo, m_url.c_str(), m_dst.c_str(), &opts);

    if (repo != nullptr) git_repository_free(repo);
    if (res != 0)
        m_pending_error = error { R"(failed to clone remote repository "{}" to "{}": {})", m_url,
                                  m_dst.c_str(), get_libgit2_error() };

    git_libgit2_shutdown();
    m_dispatch_done.emit();
}


void
cloning::mf_on_progress() const
{
    double progress;

    {
        std::lock_guard lock { m_mutex };
        progress = m_latest_progress;
    }

    m_signal_on_transfer_progress.emit(progress);
}


void
cloning::mf_on_done() const
{
    if (m_pending_error.has_value())
        m_signal_on_error.emit(m_pending_error.value());
    else
        m_signal_on_completed.emit(m_dst);
}


auto
cloning::transfer_progress_callback(const git_indexer_progress *stats, void *payload) -> int
{
    auto *self = static_cast<cloning *>(payload);

    auto received = double(stats->received_objects) / double(stats->total_objects);

    {
        std::lock_guard lock { self->m_mutex };
        self->m_latest_progress = received;
    }

    self->m_dispatch_progress.emit();
    return self->m_stop_token.stop_requested() ? -1 : 0;
}


auto
aurgh::git::clone(std::string_view url, std::filesystem::path base) noexcept
    -> result<std::shared_ptr<cloning>>
try
{
    return std::make_shared<cloning>(url, std::move(base));
}
catch (const std::exception &e)
{
    return error { R"(failed to clone remote repository "{}" to "{}": {})", url, base.c_str(),
                   e.what() }
        .unexpected();
}
catch (error &e)
{
    return e.unexpected();
}
