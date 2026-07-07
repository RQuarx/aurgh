#include "http/client.hh"
#include "http/transfer.hh"

using aurgh::http::transfer;


transfer::transfer(class client      *client,
                   std::string_view   method,
                   const std::string &url,
                   std::string        body)
    : m_client { client }, m_easy { curl_easy_init() }, m_request_body { std::move(body) }
{
    if (m_easy == nullptr) throw error { "failed to create a curl-easy handle" };

    auto set = [&](CURLoption opt, auto &&param)
    {
        if (auto res = curl_easy_setopt(m_easy.get(), opt, param); res != CURLE_OK)
            throw error { "failed to setopt for curl-easy {}", curl_easy_strerror(res) };
    };

    set(CURLOPT_URL, url.c_str());
    set(CURLOPT_WRITEFUNCTION, &write_callback);
    set(CURLOPT_WRITEDATA, this);
    set(CURLOPT_PRIVATE, this);

    if (method == "POST")
    {
        set(CURLOPT_POSTFIELDS, m_request_body.c_str());
        set(CURLOPT_POSTFIELDSIZE, m_request_body.size());
    }
}


auto
transfer::write_callback(char *ptr, std::size_t size, std::size_t nmemb, void *data) -> std::size_t
{
    static_cast<transfer *>(data)->m_signal_on_data({ ptr, size * nmemb });
    return size * nmemb;
}


auto
transfer::on_data(const data_signal::slot_type &slot) -> transfer &
{
    m_signal_on_data.connect(slot);
    return *this;
}


auto
transfer::on_complete(const complete_signal::slot_type &slot) -> transfer &
{
    m_signal_on_complete.connect(slot);
    return *this;
}


auto
transfer::on_error(const error_signal::slot_type &slot) -> transfer &
{
    m_signal_on_error.connect(slot);
    return *this;
}


auto
transfer::cancel() noexcept -> result<void>
{
    if (m_client != nullptr && m_easy) return m_client->cancel(*this);
    return {};
}

void
transfer::complete(CURLcode code)
{
    if (code != CURLE_OK)
    {
        m_signal_on_error.emit(curl_easy_strerror(code));
        return;
    }

    long status = 0;
    curl_easy_getinfo(m_easy.get(), CURLINFO_RESPONSE_CODE, &status);

    m_signal_on_complete.emit(completion {
        .curl_result = code,
        .return_code = static_cast<int>(status),
    });
}