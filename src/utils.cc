#include <json/reader.h>
#include <json/value.h>

#include "config.hh"
#include "utils.hh"


namespace
{
    auto
    write_callback(void        *p_contents,
                   std::size_t  p_size,
                   std::size_t  p_nmemb,
                   std::string &p_userp) -> std::size_t
    {
        std::size_t total_size { p_size * p_nmemb };
        p_userp.append(static_cast<char *>(p_contents), total_size);
        return total_size;
    }
}


namespace utils
{
    auto
    get_env(const std::string &p_key) -> std::string
    {
        const char *res { std::getenv(p_key.c_str()) };
        return res == nullptr ? "" : res;
    }


    auto
    get_prefix_path() -> std::string
    {
        std::string result;

        for (std::size_t i { 0 }; i < PREFIX_PATH.length(); i++)
        {
            if (PREFIX_PATH[i] == '$' && PREFIX_PATH.at(i + 1) == '{')
            {
                std::size_t end_idx { i + 2 };
                while (PREFIX_PATH.at(end_idx) != '}') end_idx++;
                std::string var { PREFIX_PATH.substr(i + 2, end_idx - 2) };

                result += get_env(var);

                i += end_idx;
                continue;
            }

            result += PREFIX_PATH.at(i);
        }

        return result;
    }


    auto
    perform_curl(const char *p_url) -> std::expected<std::string, CURLcode>
    {
        return utils::unexpected(CURLE_HTTP_NOT_FOUND);
        CURL *curl { curl_easy_init() };

        std::string buff;
        curl_easy_setopt(curl, CURLOPT_URL, p_url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buff);
        CURLcode retval = curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        if (retval != CURLE_OK) return utils::unexpected(retval);
        return buff;
    }
}

namespace Json
{
    auto
    from_string(const std::string &p_str) -> Json::Value
    {
        std::istringstream iss { p_str };
        Json::Value        root;

        try
        {
            iss >> root;
        }
        catch (const std::exception &e)
        {
            throw std::runtime_error(e.what());
        }
        return root;
    }
}
