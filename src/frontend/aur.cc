#include <curl/curl.h>
#include <glibmm.h>
#include <nlohmann/json.hpp>

#include "aur.hh"

using aurgh::aur;

namespace
{
    auto
    create_packages(const nlohmann::json &response) noexcept
        -> aurgh::result<std::vector<aurgh::package>>
    {
        using namespace aurgh;

        if (!response.is_object())
            return error { "received response is malformed (not an object)." }.unexpected();

        if (response.contains("error"))
            return error { "AUR API returned an error: {}",
                           response["error"].get<std::string_view>() }
                .unexpected();

        std::vector<package> packages;
        packages.reserve(response["resultcount"].get<std::size_t>());

        for (const auto &pkg : response["results"])
            packages.emplace_back(package::from_json(pkg));
        return packages;
    }


    auto
    create_package_infos(const nlohmann::json &response)
        -> aurgh::result<std::vector<aurgh::package_details>>
    {
        using namespace aurgh;

        if (!response.is_object())
            return error { "received response is malformed (not an object)." }.unexpected();

        if (response.contains("error"))
            return error { "AUR API returned an error: {}",
                           response["error"].get<std::string_view>() }
                .unexpected();

        std::vector<package_details> packages;
        packages.reserve(response["resultcount"].get<std::size_t>());

        for (const auto &pkg : response["results"])
            packages.emplace_back(package_details::from_json(pkg));

        return packages;
    }
}


aur::aur(const std::shared_ptr<http::client> &client) noexcept : m_client { client } {}


auto
aur::search(Glib::UStringView name) noexcept
    -> result<std::shared_ptr<request<std::vector<package>>>>
try
{
    std::string url = std::format("{}/search/{}?by=name", URL, name.c_str());

    if (auto trans = m_client->get(url); trans.has_value())
        return std::shared_ptr<request<std::vector<package>>> {
            new request<std::vector<package>> { trans.value(), create_packages }
        };
    else /* NOLINT */
        return trans.error().unexpected();
}
catch (const std::exception &e)
{
    return error { "failed to search for {}: {}", name.c_str(), e.what() }.unexpected();
}


auto
aur::info(std::span<const std::string> args) noexcept
    -> result<std::shared_ptr<request<std::vector<package_details>>>>
try
{
    std::string url = std::format("{}/info?", URL);

    for (const auto &arg : args) url += std::format("arg[]={}&", arg);
    if (url.back() == '&') url.pop_back();

    if (auto trans = m_client->get(url); trans.has_value())
        return std::shared_ptr<request<std::vector<package_details>>> {
            new request<std::vector<package_details>> { trans.value(), create_package_infos }
        };
    else /* NOLINT */
        return trans.error().unexpected();
}
catch (const std::exception &e)
{
    return error { "failed to fetch info: {}", e.what() }.unexpected();
}
