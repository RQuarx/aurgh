#pragma once
#include <expected>
#include <string>


namespace app
{
    using void_or_err = std::expected<void,    std::string>;
    using int_or_err  = std::expected<int32_t, std::string>;
}