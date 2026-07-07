#pragma once
#include <nlohmann/json.hpp>

#include "http/client.hh"
#include "package.hh"


namespace aurgh
{
    class aur
    {
        static constexpr auto URL = "https://aur.archlinux.org/rpc/v5";

    public:
        template <typename T>
        class request
        {
            friend class aur;
            using error_signal  = sigc::signal<void(error)>;
            using result_signal = sigc::signal<void(T)>;
            using parser_type   = auto (*)(const nlohmann::json &) -> result<T>;

        public:
            auto
            on_result(const result_signal::slot_type &slot) -> request &
            {
                m_signal_on_result.connect(slot);
                return *this;
            }

            auto
            on_error(const error_signal::slot_type &slot) -> request &
            {
                m_signal_on_error.connect(slot);
                return *this;
            }

        private:
            std::shared_ptr<http::transfer> m_transfer;

            std::string m_buffer;

            result_signal m_signal_on_result;
            error_signal  m_signal_on_error;
            parser_type   m_parser;


            request(const std::shared_ptr<http::transfer> &transfer, parser_type fn)
                : m_transfer { transfer }, m_parser { fn }
            {
                m_transfer->on_data([this](std::string_view data) { m_buffer.append(data); });
                m_transfer->on_complete(
                    [this](http::completion complete)
                    {
                        if (complete.curl_result != CURLE_OK)
                        {
                            m_signal_on_error.emit(
                                error { "curl has failed on the transfer: {}",
                                        curl_easy_strerror(complete.curl_result) });
                            return;
                        }

                        if (complete.return_code != 200)
                        {
                            m_signal_on_error.emit(
                                error { "AUR has returned code {}", complete.return_code });
                            return;
                        }

                        nlohmann::json response;

                        try
                        {
                            response = nlohmann::json::parse(m_buffer);
                        }
                        catch (const std::exception &e)
                        {
                            m_signal_on_error.emit(
                                error { "failed to parse json response: {}", e.what() });
                            return;
                        }

                        if (auto res = m_parser(response); res.has_value())
                            m_signal_on_result.emit(std::move(res.value()));
                        else
                            m_signal_on_error.emit(res.error());
                    });
                m_transfer->on_error(
                    [this](std::string_view e)
                    { m_signal_on_error.emit(error { "transfer failer: {}", e }); });
            }
        };


        aur(const std::shared_ptr<http::client> &client) noexcept;


        [[nodiscard]]
        auto search(Glib::UStringView name) noexcept
            -> result<std::shared_ptr<request<std::vector<package>>>>;

        [[nodiscard]]
        auto info(std::span<const std::string> args) noexcept
            -> result<std::shared_ptr<request<std::vector<package_details>>>>;

    private:
        std::shared_ptr<http::client> m_client;
    };
}
