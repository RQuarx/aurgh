#pragma once
#include <memory>
#include <vector>

#include "log.hh"

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
        add_keyword(const std::string &p_keyword)
        {
            keywords.emplace_back(p_keyword);
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
         * @param p_logger   A Logger std::shared_ptr instance.
         * @param p_pkg_name The name of the package.
         * @param p_system   Whether to use the AUR or libalpm.
         */
        Package(const std::shared_ptr<Logger> &p_logger,
                const std::string             &p_pkg_name,
                bool                           p_system = false);

        /**
         * @brief This ctor will accept a Json::Value object instead of creating
         *        a GET request to find the information about a package.
         */
        Package(const std::shared_ptr<Logger> &p_logger,
                const Json::Value             &p_pkg_info);

        [[nodiscard]]
        auto operator[](PkgInfo p_info) -> std::string &;

        [[nodiscard]]
        auto get_keywords() const -> const std::vector<std::string> &;

        [[nodiscard]]
        auto is_valid() const -> bool;

    private:
        std::shared_ptr<Logger> m_logger;
        bool                    m_valid;
        Pkg                     m_pkg;

        [[nodiscard]]
        auto json_to_pkg(const Json::Value &p_json) -> bool;
    };
}
