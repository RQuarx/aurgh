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
    add_keyword(std::string &&keyword)
    {
        keywords.emplace_back(std::move(keyword));
    }


    [[nodiscard]]
    auto
    operator[](PkgInfo type) -> std::string &
    {
        return info.at(type);
    }
};


class Package
{
public:
    Package(const Json::Value &pkg_info, bool from_aur = true);


    [[nodiscard]]
    auto operator[](PkgInfo info) -> std::string &;


    [[nodiscard]]
    auto get_keywords() const -> const std::vector<std::string> &;


    [[nodiscard]]
    auto is_valid() const -> bool;


    [[nodiscard]]
    auto is_external() const -> bool;


    [[nodiscard]]
    auto get_error_message() const -> std::string;


    [[nodiscard]]
    static auto get_installed()
        -> std::expected<std::vector<Package>, std::string>;

private:
    bool valid;
    Pkg  pkg;
    bool from_aur;

    std::string error_message;


    [[nodiscard]]
    auto json_to_pkg(const Json::Value &json) -> bool;
};
