#include <alpm.h>
#include <glibmm.h>
#include <sys/utsname.h>

#include "http/client.hh"

using aurgh::http::client;

namespace
{
    auto
    set_user_agent() -> aurgh::result<void>
    {
        struct utsname un;

        if (uname(&un) == -1)
            return aurgh::error { "uname failed: {}", strerror(errno) }.unexpected();

        std::string agent = std::format("aurgh/{} ({} {}) libalpm/{}", PROJECT_VERSION, un.sysname,
                                        un.machine, alpm_version());
        setenv("HTTP_USER_AGENT", agent.c_str(), 0);
        return {};
    }
}


auto
client::create() noexcept -> result<std::shared_ptr<client>>
try
{
    if (auto res = set_user_agent(); !res.has_value()) return res.error().unexpected();
    return std::shared_ptr<client> { new client {} };
}
catch (const std::exception &e)
{
    return error { "failed to create an http client: {}", e.what() }.unexpected();
}
catch (error &err)
{
    return err.unexpected();
}


client::client() : m_multi { curl_multi_init() }
{
    if (m_multi == nullptr) throw error { "failed to create the curl-multi handle" };

    CURLMcode res;

    if (res = curl_multi_setopt(m_multi, CURLMOPT_SOCKETFUNCTION, client::socket_cb);
        res != CURLM_OK)
        throw error { "failed to setopt for curl-multi: {}", curl_multi_strerror(res) };
    if (res = curl_multi_setopt(m_multi, CURLMOPT_SOCKETDATA, this); res != CURLM_OK)
        throw error { "failed to setopt for curl-multi: {}", curl_multi_strerror(res) };
    if (res = curl_multi_setopt(m_multi, CURLMOPT_TIMERFUNCTION, client::timer_cb); res != CURLM_OK)
        throw error { "failed to setopt for curl-multi: {}", curl_multi_strerror(res) };
    if (res = curl_multi_setopt(m_multi, CURLMOPT_TIMERDATA, this); res != CURLM_OK)
        throw error { "failed to setopt for curl-multi: {}", curl_multi_strerror(res) };
}


client::~client()
{
    if (m_multi != nullptr) curl_multi_cleanup(m_multi);
}


client::client(client &&other) noexcept
    : m_multi { std::exchange(other.m_multi, nullptr) }, m_watches { std::move(other.m_watches) },
      m_still_running { std::exchange(other.m_still_running, 0) },
      m_timer_connection { other.m_timer_connection }, m_transfers { std::move(other.m_transfers) }
{
    other.m_timer_connection.disconnect();

    if (m_multi != nullptr)
    {
        curl_multi_setopt(m_multi, CURLMOPT_SOCKETDATA, this);
        curl_multi_setopt(m_multi, CURLMOPT_TIMERDATA, this);
    }

    for (auto &[_, t] : m_transfers) t->m_client = this;
}


auto
client::operator=(client &&other) noexcept -> client &
{
    if (this != &other)
    {
        if (m_multi != nullptr) curl_multi_cleanup(m_multi);

        m_multi            = std::exchange(other.m_multi, nullptr);
        m_watches          = std::move(other.m_watches);
        m_still_running    = std::exchange(other.m_still_running, 0);
        m_timer_connection = other.m_timer_connection;
        m_transfers        = std::move(other.m_transfers);

        other.m_timer_connection.disconnect();

        if (m_multi != nullptr)
        {
            curl_multi_setopt(m_multi, CURLMOPT_SOCKETDATA, this);
            curl_multi_setopt(m_multi, CURLMOPT_TIMERDATA, this);
        }

        for (auto &[_, t] : m_transfers) t->m_client = this;
    }

    return *this;
}


auto
client::on_io_event(Glib::IOCondition cond, curl_socket_t fd, client *client) -> bool
{
    using enum Glib::IOCondition;

    int action = 0;
    if (std::to_underlying(cond & IO_IN) != 0) action |= CURL_CSELECT_IN;
    if (std::to_underlying(cond & IO_OUT) != 0) action |= CURL_CSELECT_OUT;
    if (std::to_underlying(cond & IO_ERR) != 0) action |= CURL_CSELECT_ERR;

    curl_multi_socket_action(client->m_multi, fd, action, &client->m_still_running);
    check_completed(client);
    return true;
}


auto
client::socket_cb(CURL * /* easy */, curl_socket_t fd, int what, void *userp, void * /* sockp */)
    -> int
{
    auto *client = static_cast<class client *>(userp);

    if (what == CURL_POLL_REMOVE)
    {
        if (auto it = client->m_watches.find(fd); it != client->m_watches.end())
        {
            it->second.disconnect();
            client->m_watches.erase(it);
        }

        return 0;
    }

    using enum Glib::IOCondition;
    Glib::IOCondition condition = IO_ERR | IO_HUP;

    if (what == CURL_POLL_IN || what == CURL_POLL_INOUT) condition |= IO_IN;
    if (what == CURL_POLL_OUT || what == CURL_POLL_INOUT) condition |= IO_OUT;

    auto &watch = client->m_watches[fd];
    watch.disconnect();
    watch = Glib::signal_io().connect([client, fd](Glib::IOCondition c)
                                      { return on_io_event(c, fd, client); }, fd, condition);
    return 0;
}


auto
client::timer_cb(CURLM * /* multi */, long timeout_ms, void *userp) -> int
{
    auto *client = static_cast<class client *>(userp);

    client->m_timer_connection.disconnect();

    if (timeout_ms < 0) return 0;

    client->m_timer_connection = Glib::signal_timeout().connect(
        [client]
        {
            curl_multi_socket_action(client->m_multi, CURL_SOCKET_TIMEOUT, 0,
                                     &client->m_still_running);
            check_completed(client);
            return false;
        },
        static_cast<guint>(std::max(1L, timeout_ms)));

    return 0;
}


void
client::check_completed(client *c)
{
    CURLMsg *msg;
    int      msgs_left;

    while ((msg = curl_multi_info_read(c->m_multi, &msgs_left)) != nullptr)
    {
        if (msg->msg != CURLMSG_DONE) continue;

        if (auto it = c->m_transfers.find(msg->easy_handle); it != c->m_transfers.end())
        {
            std::shared_ptr<transfer> t = std::move(it->second);
            c->m_transfers.erase(it);

            curl_multi_remove_handle(c->m_multi, t->m_easy.get());
            t->complete(msg->data.result);
        }
    }
}


auto
client::cancel(transfer &t) noexcept -> result<void>
{
    if (t.m_client == nullptr) return {};

    curl_multi_remove_handle(m_multi, t.m_client);
    m_transfers.erase(t.m_client);
    t.m_easy.reset();
    t.m_client = nullptr;

    return {};
}


auto
client::get(const std::string &url) noexcept -> result<std::shared_ptr<transfer>>
{ return mf_add_transfer(url, "GET"); }


auto
client::post(const std::string &url, std::string body) noexcept -> result<std::shared_ptr<transfer>>
{ return mf_add_transfer(url, "POST", std::move(body)); }


auto
client::mf_add_transfer(const std::string &url, std::string_view method, std::string body) noexcept
    -> result<std::shared_ptr<transfer>>
try
{
    std::shared_ptr<transfer> trans {
        new transfer { this, method, url, std::move(body) }
    };

    if (CURLMcode res = curl_multi_add_handle(m_multi, trans->m_easy.get()); res != CURLM_OK)
        return error { "failed to add easy handle: {}", curl_multi_strerror(res) }.unexpected();

    m_transfers[trans->m_easy.get()] = trans;
    return trans;
}
catch (const std::exception &e)
{
    return error { "failed to create a transfer for \"{}\"", url }.unexpected();
}
catch (error &e)
{
    return e.unexpected();
}
