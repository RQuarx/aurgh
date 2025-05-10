/**
 * aurgh Copyright (C) 2025 RQuarx
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
#include <vector>

using str_pair = std::pair<std::string, std::string>;


namespace pkg {
    enum Type : int8_t
    {
        Install = -1,
        Remove  = 0,
        Update  = 1,
        None    = 2,
    };

    struct Actions
    {
        std::vector<std::string> install;
        std::vector<std::string> remove;
        std::vector<std::string> update;

        Actions()
        {
            install.reserve(10);
            remove.reserve(10);
            update.reserve(10);
        }

        [[nodiscard]]
        auto at(pkg::Type t) -> std::vector<std::string>*
        {
            switch (t)
            {
            case pkg::Install: return &install;
            case pkg::Remove:  return &remove;
            case pkg::Update:  return &update;
            case pkg::None:    return nullptr;
            default:           return &install;
            }
        }
    };
} /* namespace pkg */

template<>
struct std::formatter<pkg::Type> : std::formatter<std::string>
{
    static auto
    format(pkg::Type type, format_context &ctx)
    {
        switch (type)
        {
        case pkg::Install: return format_to(ctx.out(), "install");
        case pkg::Remove:  return format_to(ctx.out(), "remove");
        case pkg::Update:  return format_to(ctx.out(), "update");
        default:           return format_to(ctx.out(), "none");
        }
    }
};

template <typename CharT>
struct std::formatter<std::vector<std::string>, CharT>
{
    constexpr auto
    parse(std::format_parse_context &ctx)
    { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const std::vector<std::string> &vec, FormatContext &ctx) const
    {
        auto out = ctx.out();
        *out++ = "\n\t[";

        if (!vec.empty()) {
            *out++ = '\n';
            for (size_t i = 0; i < vec.size(); ++i) {
                out = std::format_to(
                    out,
                    "\t\t\"{}\"{}",
                    vec.at(i),
                    (i + 1 < vec.size() ? ",\n" : "\n")
                );
            }
            out = std::format_to(out, "\t");
        }

        *out++ = ']';
        return out;
    }
};

template <typename CharT>
struct std::formatter<pkg::Actions, CharT>
{
    constexpr auto
    parse(std::format_parse_context &ctx)
    { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const pkg::Actions &actions, FormatContext &ctx) const
    {
        auto out = ctx.out();
        out = std::format_to(out,
"\"actions\":\n{{\n\t\"install\":\t{},\n\t\"remove\":\t{},\n\t\"update\":\t{}\n}}",
            actions.install, actions.remove, actions.update);
        return out;
    }
};


#endif /* package/type.hh */