#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace Json { class Value; }


namespace app
{
    enum PkgInfo : uint8_t
    {
        PKG_NAME       = 0,
        PKG_VERSION    = 1,
        PKG_MAINTAINER = 2,
        PKG_DESC       = 3,
        PKG_URL        = 4,
        PKG_NUMVOTES   = 5
    };

    struct Pkg
    {
        std::array<std::string, 6> info;
        std::vector<std::string>   keywords;
        bool                       installed { false };

        Pkg() { keywords.reserve(10); }


        void
        add_keyword(std::string &&p_keyword)
        {
            keywords.emplace_back(std::move(p_keyword));
        }


        [[nodiscard]]
        auto
        operator[](PkgInfo p_type) -> std::string &
        {
            return info.at(p_type);
        }
    };


    class Package
    {
    public:
        /**
         * The ctor will get information of the package @p pkg_name from the AUR,
         * if @p system is false, or from libalpm if @p system is true.
         *
         * @param p_pkg_name The name of the package.
         * @param p_system   Whether to use the AUR or libalpm.
         */
        Package(const std::string &p_pkg_name, bool p_system = false);


        /**
         * @brief This ctor will accept a Json::Value object instead of creating
         *        a GET request to find the information about a package.
         */
        Package(const Json::Value &p_pkg_info);


        [[nodiscard]]
        auto operator[](this Package &self, PkgInfo p_info) -> std::string &;


        [[nodiscard]]
        auto get_keywords(this const Package &self)
            -> const std::vector<std::string> &;


        [[nodiscard]]
        auto is_valid(this const Package &self) -> bool;

    private:
        bool valid;
        Pkg  pkg;

        [[nodiscard]]
        auto json_to_pkg(this Package &self, const Json::Value &p_json) -> bool;
    };
}
