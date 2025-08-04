#pragma once
#include <expected>
#include <cstdint>
#include <string>

namespace Glib
{
    template<typename T_CppObject>
    class RefPtr;
}
namespace Gtk { class Builder; }

using str_pair = std::pair<std::string, std::string>;

using builder_t = Glib::RefPtr<Gtk::Builder>;


template<typename T>
using res_or_string = std::expected<T, std::string>;

using int8_t  = std::int8_t;
using int16_t = std::int16_t;
using int32_t = std::int32_t;
using int64_t = std::int64_t;

using uint8_t  = std::uint8_t;
using uint16_t = std::uint16_t;
using uint32_t = std::uint32_t;
using uint64_t = std::uint64_t;

using size_t = std::size_t;
using ssize_t = std::int64_t;