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


    struct libgit2_session
    {
        libgit2_session()
        {
            if (git_libgit2_init() < 0)
                throw aurgh::error { "failed to initialize libgit2: {}", get_libgit2_error() };
        }

        ~libgit2_session() noexcept { git_libgit2_shutdown(); }
    };
}


cloning::cloning(std::string url, std::filesystem::path to)
    : m_url { std::move(url) }, m_to { std::move(to) }
{
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

    try
    {
        libgit2_session session;
    }
    catch (error &e)
    {
        m_pending_error = std::move(e);
        m_dispatch_done.emit();

        return;
    }

    git_clone_options opts;
    git_clone_options_init(&opts, GIT_CLONE_OPTIONS_VERSION);

    opts.fetch_opts.callbacks.transfer_progress = &cloning::transfer_progress_callback;
    opts.fetch_opts.callbacks.payload           = this;

    git_repository *repo = nullptr;
    int             res  = git_clone(&repo, m_url.c_str(), m_to.c_str(), &opts);

    if (repo != nullptr) git_repository_free(repo);
    if (res != 0)
        m_pending_error = error { R"(failed to clone remote repository "{}" to "{}": {})", m_url,
                                  m_to.c_str(), get_libgit2_error() };

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
        m_signal_on_completed.emit();
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
aurgh::git::clone(std::string url, std::filesystem::path to) noexcept
    -> result<std::shared_ptr<cloning>>
try
{
    return std::make_shared<cloning>(std::move(url), std::move(to));
}
catch (const std::exception &e)
{
    return error { R"(failed to clone remote repository "{}" to "{}": {})", url, to.c_str(),
                   e.what() }
        .unexpected();
}
catch (error &e)
{
    return e.unexpected();
}
