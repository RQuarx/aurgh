#pragma once
#include <memory>
#include <vector>
#include "log.hh"

namespace Json { class Value; }


enum PkgInfo
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
    std::vector<std::string> keywords;
    bool installed;

    Pkg( void ) : installed(false)
    { keywords.reserve(10); }


    void
    add_keyword( const std::string &kw )
    { keywords.emplace_back(kw); }


    [[nodiscard]]
    auto operator[]( PkgInfo type ) -> std::string &
    { return info.at(type); }
};


class Package
{
public:
    /**
     * The ctor will get information of the package @p pkg_name from the AUR,
     * if @p system is false, or from libalpm if @p system is true.
     *
     * @param logger   A Logger std::shared_ptr instance.
     * @param pkg_name The name of the package.
     * @param system   Whether to use the AUR or libalpm.
     */
    Package( const std::shared_ptr<Logger> &logger,
             const std::string             &pkg_name,
             bool                           system = false );

    /**
     * @brief This ctor will accept a Json::Value object instead of creating
     *        a GET request to find the information about a package.
     */
    Package( const std::shared_ptr<Logger> &logger,
             const Json::Value             &pkg_info );

    [[nodiscard]]
    auto operator[]( PkgInfo info ) -> std::string &;

    [[nodiscard]]
    auto get_keywords( void ) -> const std::vector<std::string> &;

    [[nodiscard]]
    auto is_valid( void ) -> bool;

private:
    std::shared_ptr<Logger> m_logger;
    bool m_valid;
    Pkg  m_pkg;

    [[nodiscard]]
    auto json_to_pkg( const Json::Value &json ) -> bool;
};