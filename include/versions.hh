#pragma once
#include <string>


namespace versions
{
    [[nodiscard]]
    auto get(const std::string &p_name) -> std::string;
}
