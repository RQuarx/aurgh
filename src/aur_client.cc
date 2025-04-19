#include <json/json.h>
#include "aur_client.hh"
#include "logger.hh"


AUR_Client::AUR_Client(Logger *logger, std::string_view url) :
    m_url(url), m_curl(curl_easy_init()), m_logger(logger)
{
    m_logger->log(Logger::Debug, "AUR Client instance successfully created");

    if (m_curl == nullptr) {
        m_logger->log(Logger::Error, "Failed to init easy curl");
    }
}


AUR_Client::~AUR_Client()
{ curl_easy_cleanup(m_curl); }


auto
AUR_Client::perform_curl(
    const std::string &url, std::string &read_buffer
) -> CURLcode
{
    curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, Utils::write_callback);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &read_buffer);
    return curl_easy_perform(m_curl);
}


auto
AUR_Client::get_json_from_stream(std::istringstream &iss) -> Json::Value
{
    Json::CharReaderBuilder reader;
    Json::Value data;
    std::string errs;
    if (!Json::parseFromStream(reader, iss, &data, &errs)) {
        m_logger->log(
            Logger::Error,
            "Failed to parse Json from stream: {}", errs
        );
    }
    return data;
}


auto
AUR_Client::search(const std::string &args, const std::string &by) -> Json::Value
{
    std::string full_url = std::format("{}/search/{}", m_url, args);
    if (!by.empty()) {
        full_url.append("?by" + by);
    }

    std::string read_buffer;
    perform_curl(full_url, read_buffer);

    std::istringstream iss(read_buffer);
    return get_json_from_stream(iss);
}


auto
AUR_Client::info(const std::string &args) -> Json::Value
{
    std::string full_url = std::format("{}/info?arg[]={}", m_url, args);

    std::string read_buffer;
    perform_curl(full_url, read_buffer);

    std::istringstream iss(read_buffer);
    return get_json_from_stream(iss);
}