#include <memory>

#include "client.hh"
#include "result.hh"

using aurgh::client;


auto
client::create(const std::shared_ptr<http::client> &http,
               std::filesystem::path                clone_dir,
               const std::filesystem::path &pacman_conf) noexcept -> result<std::unique_ptr<client>>
try
{
    if (auto res = alpm::handle::create(pacman_conf); res.has_value())
        return std::unique_ptr<client> {
            new client { http, std::move(clone_dir), std::move(res.value()) }
        };
    else /* NOLINT */
        return res.error().unexpected();
}
catch (error &e)
{
    return e.unexpected();
}
catch (const std::exception &e)
{
    return error { "failed to create a new client: {}", e.what() }.unexpected();
}


client::client(const std::shared_ptr<http::client> &http,
               std::filesystem::path              &&clone_dir,
               alpm::handle                       &&handle)
    : m_client { http }, m_clone_dir { std::move(clone_dir) }, m_aur { m_client },
      m_alpm { std::move(handle) }
{
}


auto
client::search(const std::string &query) noexcept -> result<void>
try
{
    return m_search_operation.perform(query, m_aur, m_alpm, &aur::search, &alpm::async::search);
}
catch (const std::exception &e)
{
    return error { "failed to perform search of \"{}\": {}", query.c_str(), e.what() }.unexpected();
}


auto
client::info(const std::vector<std::string> &args) noexcept -> result<void>
try
{
    return m_info_operation.perform(args, m_aur, m_alpm, &aur::info, &alpm::async::info);
}
catch (const std::exception &e)
{
    return error { "failed to perform info retrieval for {} packages: {}", args.size(), e.what() }
        .unexpected();
}


auto
client::clone(std::string_view url) noexcept -> result<std::reference_wrapper<clone_process>>
try
{
    if (auto res = git::clone(url, m_clone_dir); res.has_value())
    {
        std::scoped_lock lock { m_clone_mutex };
        return m_clones.emplace_back(std::move(res.value()));
    }
    else /* NOLINT */
        return res.error().unexpected();
}
catch (const std::exception &e)
{
    return error { "failed to perform git clone for remote repository \"{}\": {}", url, e.what() }
        .unexpected();
}


auto
client::signal_on_search_complete() const -> sigc::signal<void(result<std::vector<package>>)>
{ return m_search_operation.signal; }


auto
client::signal_on_info_complete() const -> sigc::signal<void(result<std::vector<package_details>>)>
{ return m_info_operation.signal; }


using clone_process = client::clone_process;


clone_process::clone_process(std::shared_ptr<git::cloning> &&cloning)
    : m_cloning { std::move(cloning) }
{
    m_complete_dispatcher.connect([this] { m_signal_on_complete.emit(m_dst); });
    m_progress_dispatcher.connect([this] { m_signal_on_progress.emit(m_progress); });

    m_cloning
        ->on_transfer_progress(
            [this](double progress)
            {
                m_progress.store(progress, std::memory_order_relaxed);
                m_progress_dispatcher.emit();
            })
        .on_completed(
            [this](std::filesystem::path dst)
            {
                m_dst = std::move(dst);
                m_complete_dispatcher.emit();
            })
        .on_error(
            [this](error e)
            {
                m_dst = e.unexpected();
                m_complete_dispatcher.emit();
            });
}


clone_process::clone_process(clone_process &&other) noexcept
    : m_signal_on_complete { std::move(other.m_signal_on_complete) },
      m_signal_on_progress { std::move(other.m_signal_on_progress) },
      m_dst { std::move(other.m_dst) }, m_progress { other.m_progress.load() },
      m_cloning { std::move(other.m_cloning) }
{
    m_complete_dispatcher.connect([this] { m_signal_on_complete.emit(m_dst); });
    m_progress_dispatcher.connect([this] { m_signal_on_progress.emit(m_progress); });

    m_cloning
        ->on_transfer_progress(
            [this](double progress)
            {
                m_progress.store(progress, std::memory_order_relaxed);
                m_progress_dispatcher.emit();
            })
        .on_completed(
            [this](std::filesystem::path dst)
            {
                m_dst = std::move(dst);
                m_complete_dispatcher.emit();
            })
        .on_error(
            [this](error e)
            {
                m_dst = e.unexpected();
                m_complete_dispatcher.emit();
            });
}


auto
clone_process::operator=(clone_process &&other) noexcept -> clone_process &
{
    if (this == &other) return *this;

    m_signal_on_complete = std::move(other.m_signal_on_complete);
    m_signal_on_progress = std::move(other.m_signal_on_progress);
    m_dst                = std::move(other.m_dst);
    m_progress           = other.m_progress.load();
    m_cloning            = std::move(other.m_cloning);

    m_complete_dispatcher.connect([this] { m_signal_on_complete.emit(m_dst); });
    m_progress_dispatcher.connect([this] { m_signal_on_progress.emit(m_progress); });

    m_cloning
        ->on_transfer_progress(
            [this](double progress)
            {
                m_progress.store(progress, std::memory_order_relaxed);
                m_progress_dispatcher.emit();
            })
        .on_completed(
            [this](std::filesystem::path dst)
            {
                m_dst = std::move(dst);
                m_complete_dispatcher.emit();
            })
        .on_error(
            [this](error e)
            {
                m_dst = e.unexpected();
                m_complete_dispatcher.emit();
            });

    return *this;
}


auto
clone_process::signal_on_clone_complete() const -> sigc::signal<void(result<std::filesystem::path>)>
{ return m_signal_on_complete; }


auto
clone_process::signal_on_clone_progress() const -> sigc::signal<void(double)>
{ return m_signal_on_progress; }

