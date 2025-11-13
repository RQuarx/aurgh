#include <json/reader.h>
#include <json/value.h>

#include "config.hh"
#include "utils.hh"


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
