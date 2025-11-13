#pragma once
#include <string>
#include <unordered_map>


namespace versions
{
    [[nodiscard]]
    auto get(const std::string &p_name) -> std::string;


    [[nodiscard]]
    auto get() -> std::unordered_map<std::string, std::string>;
}
