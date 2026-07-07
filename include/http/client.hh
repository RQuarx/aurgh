#pragma once
#include <map>

#include "transfer.hh"


namespace aurgh::http
{
    class client
    {
    public:
        ~client();
        client(const client &)                     = delete;
        auto operator=(const client &) -> client & = delete;
        client(client &&other) noexcept;
        auto operator=(client &&other) noexcept -> client &;


        [[nodiscard]] static auto create() noexcept -> result<std::shared_ptr<client>>;


        [[nodiscard]]
        auto get(const std::string &url) noexcept -> result<std::shared_ptr<transfer>>;


        [[nodiscard]]
        auto post(const std::string &url, std::string body) noexcept
            -> result<std::shared_ptr<transfer>>;


        auto cancel(transfer &t) noexcept -> result<void>;

    private:
        CURLM *m_multi;

        std::map<curl_socket_t, sigc::connection> m_watches;

        int              m_still_running = 0;
        sigc::connection m_timer_connection;

        std::map<CURL *, std::shared_ptr<transfer>> m_transfers;


        static auto on_io_event(Glib::IOCondition cond, curl_socket_t fd, client *client) -> bool;
        static auto socket_cb(CURL *easy, curl_socket_t fd, int what, void *userp, void *sockp)
            -> int;
        static auto timer_cb(CURLM *multi, long timeout_ms, void *userp) -> int;
        static void check_completed(client *client);


        auto mf_add_transfer(const std::string &url,
                             std::string_view   method,
                             std::string body = {}) noexcept -> result<std::shared_ptr<transfer>>;


        client();
    };
}
