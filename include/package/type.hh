/**
 * @file package/type.hh
 *
 * This file is part of aurgh
 *
 * aurgh is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * aurgh is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with aurgh. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#ifndef PACKAGE_TYPE_HH__
#define PACKAGE_TYPE_HH__

#include <cstdint>
#include <utility>
#include <format>
#include <string>

using str_pair = std::pair<std::string, std::string>;


namespace pkg {
    enum Type : int8_t {
        Install = -1,
        Remove  = 0,
        Update  = 1,
        None    = 2,
    };
} /* namespace pkg */

template<>
struct std::formatter<pkg::Type> : std::formatter<std::string> {
    static auto format(pkg::Type type, format_context& ctx) {
        switch (type) {
            case pkg::Install: return format_to(ctx.out(), "install");
            case pkg::Remove:  return format_to(ctx.out(), "remove");
            case pkg::Update:  return format_to(ctx.out(), "update");
            default:      return format_to(ctx.out(), "none");
        }
    }
};


#endif /* package/type.hh */