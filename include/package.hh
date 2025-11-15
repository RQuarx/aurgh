#pragma once
#include <array>
#include <cstdint>
#include <expected>
#include <string>
#include <vector>

namespace Json { class Value; }


enum PkgInfo : std::uint8_t
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
    Package(const Json::Value &p_pkg_info, bool p_from_aur = true);


    [[nodiscard]]
    auto operator[](this Package &self, PkgInfo p_info) -> std::string &;


    [[nodiscard]]
    auto get_keywords(this const Package &self)
        -> const std::vector<std::string> &;


    [[nodiscard]]
    auto is_valid(this const Package &self) -> bool;


    [[nodiscard]]
    auto is_external(this const Package &self) -> bool;


    [[nodiscard]]
    auto get_error_message(this const Package &self) -> std::string;


    [[nodiscard]]
    static auto get_installed()
        -> std::expected<std::vector<Package>, std::string>;

private:
    bool valid;
    Pkg  pkg;
    bool from_aur;

    std::string error_message;


    [[nodiscard]]
    auto json_to_pkg(this Package &self, const Json::Value &p_json) -> bool;
};
