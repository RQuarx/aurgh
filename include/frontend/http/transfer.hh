#pragma once
#include <curl/multi.h>
#include <glibmm/iochannel.h>
#include <sigc++/connection.h>

#include "result.hh"
#include "utils.hh"


namespace aurgh::http
{
    struct completion
    {
        CURLcode curl_result;
        int      return_code;
    };


    class transfer final : public sigc::trackable
    {
        friend class client;

        using easy_destructor = util::destructor<CURL, curl_easy_cleanup>;

        using data_signal     = sigc::signal<void(std::string_view)>;
        using complete_signal = sigc::signal<void(completion)>;
        using error_signal    = sigc::signal<void(std::string_view)>;

    public:
        auto on_data(const data_signal::slot_type &slot) -> transfer &;
        auto on_complete(const complete_signal::slot_type &slot) -> transfer &;
        auto on_error(const error_signal::slot_type &slot) -> transfer &;

        auto cancel() noexcept -> result<void>;

    private:
        class client *m_client;

        std::unique_ptr<CURL, easy_destructor> m_easy;
        std::string                            m_request_body;

        data_signal     m_signal_on_data;
        complete_signal m_signal_on_complete;
        error_signal    m_signal_on_error;


        transfer(class client      *client,
                 std::string_view   method,
                 const std::string &url,
                 std::string        body = {});


        static auto write_callback(char *p, std::size_t s, std::size_t n, void *d) -> std::size_t;
        void        complete(CURLcode code);
    };
}
