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
#ifndef __PACKAGE__TYPE__HH
#define __PACKAGE__TYPE__HH

#include <unordered_set>
#include <utility>
#include <format>
#include <memory>
#include <vector>
#include <glibmm/refptr.h>
#include <alpm.h>
#include "types.hh"

namespace Gtk { class Builder; }
class Logger;

using str_pair      = std::pair<str, str>;
using str_vec       = std::vector<str>;
using str_pair_vec  = std::vector<str_pair>;
using builder_t     = Glib::RefPtr<Gtk::Builder>;
using pkg_uset      = std::unordered_set<alpm_pkg_t*>;


namespace pkg {
    class Client;

    static constexpr usize DEFAULT_RESERVE_SIZE = 10ZU;

    enum Type : i8
    {
        Install = -1,
        Remove  = 0,
        Update  = 1,
        None    = 2,
    };


    struct Actions
    {
        shared_ptr<str_vec> install;
        shared_ptr<str_vec> remove;
        shared_ptr<str_vec> update;

        Actions() :
            install(std::make_shared<str_vec>()),
            remove(std::make_shared<str_vec>()),
            update(std::make_shared<str_vec>())
        {
            install->reserve(DEFAULT_RESERVE_SIZE);
            remove->reserve(DEFAULT_RESERVE_SIZE);
            update->reserve(DEFAULT_RESERVE_SIZE);
        }

        [[nodiscard]]
        auto at(pkg::Type t) const -> shared_ptr<str_vec>
        {
            switch (t)
            {
            case pkg::Install: return install;
            case pkg::Remove:  return remove;
            case pkg::Update:  return update;
            case pkg::None:    return install;
            }
            return install;
        }
    };


    struct CardData
    {
        str                  card_builder_file;
        shared_ptr<pkg_uset> installed_pkgs;
        shared_ptr<Actions>  actions;
    };
} /* namespace pkg */

template<>
struct std::formatter<pkg::Type> : std::formatter<str>
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
struct std::formatter<str_vec, CharT>
{
    constexpr auto
    parse(std::format_parse_context &ctx)
    { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const str_vec &vec, FormatContext &ctx) const
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

#endif /* __PACKAGE__TYPE__HH*/