#pragma once
#include <glibmm/ustring.h>

#include "aur.hh"
#include "git.hh"
#include "result.hh"


namespace aurgh
{
    class client
    {
    public:
        [[nodiscard]]
        static auto create() noexcept -> result<client>;


        void search(Glib::UStringView query) noexcept;


    private:
        std::shared_ptr<http::client> m_client;

        aur m_aur_client;


        std::vector<std::shared_ptr<git::cloning>>                               m_clones;
        std::vector<std::shared_ptr<aur::request<std::vector<package>>>>         m_searches;
        std::vector<std::shared_ptr<aur::request<std::vector<package_details>>>> m_infos;
    };
}
