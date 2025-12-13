#pragma once
#include <array>
#include <cstdint>
#include <expected>
#include <string>
#include <utility>
#include <vector>

namespace Json { class Value; }


enum class PackageField : std::uint8_t
{
    NAME       = 0,
    VERSION    = 1,
    MAINTAINER = 2,
    DESC       = 3,
    URL        = 4,
    NUMVOTES   = 5
};

struct PackageData
{
    std::array<std::string, 6> attributes;
    std::vector<std::string>   keywords;
    bool                       installed { false };

    PackageData() { keywords.reserve(10); }


    void
    add_keyword(std::string &&keyword)
    {
        keywords.emplace_back(std::move(keyword));
    }


    [[nodiscard]]
    auto
    operator[](PackageField field) -> std::string &
    {
        return attributes.at(std::to_underlying(field));
    }
};


class PackageEntry
{
public:
    PackageEntry(const Json::Value &pkg_info, bool from_aur = true);


    [[nodiscard]]
    auto operator[](PackageField field) -> std::string &;


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
        -> std::expected<std::vector<PackageEntry>, std::string>;

private:
    bool        m_is_valid;
    PackageData m_data;
    bool        m_is_from_aur;

    std::string m_error_message;


    [[nodiscard]]
    auto json_to_pkg(const Json::Value &json) -> bool;
};
