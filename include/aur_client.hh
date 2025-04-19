#pragma once
#ifndef AUR_CLIENT_HH__
#define AUR_CLIENT_HH__

#include <string_view>
#include <string>
#include <curl/curl.h>

namespace Json { class Value; }
class Logger;



class AUR_Client
{
public:
    explicit AUR_Client(Logger *logger, std::string_view url = "https://aur.archlinux.org/rpc/v5");
    ~AUR_Client();

    auto search(const std::string &args, const std::string &by = "") -> Json::Value;
    auto info(const std::string &args) -> Json::Value;

private:
    std::string_view m_url = "https://aur.archlinux.org/rpc/v5";

    CURL *m_curl;

    Logger *m_logger;

    auto perform_curl(const std::string &url, std::string &read_buffer) -> CURLcode;
    auto get_json_from_stream(std::istringstream &iss) -> Json::Value;
};

#endif /* aur_client.hh */