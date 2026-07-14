#include <memory>

#include "client.hh"
#include "result.hh"

using aurgh::client;


auto
client::create(const std::shared_ptr<http::client> &http,
               const std::filesystem::path &pacman_conf) noexcept -> result<std::unique_ptr<client>>
try
{
    if (auto res = alpm::handle::create(pacman_conf); res.has_value())
        return std::unique_ptr<client> {
            new client { http, std::move(res.value()) }
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


client::client(const std::shared_ptr<http::client> &http, alpm::handle &&handle)
    : m_client { http }, m_aur { m_client }, m_alpm { std::move(handle) }
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
client::signal_on_search_complete() const -> sigc::signal<void(result<std::vector<package>>)>
{ return m_search_operation.signal; }


auto
client::signal_on_info_complete() const -> sigc::signal<void(result<std::vector<package_details>>)>
{ return m_info_operation.signal; }
