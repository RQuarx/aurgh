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
#ifndef __TYPES__HH
#define __TYPES__HH

#include <unordered_set>
#include <unordered_map>
#include <json/value.h>

using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using usize = std::size_t;
using ssize = std::int64_t;

using json     = Json::Value;
using str      = std::string;
using str_view = std::string_view;

template<typename T1, typename T2>
using pair = std::pair<T1, T2>;

template<typename Key, typename Tp>
using umap = std::unordered_map<Key, Tp>;

template<typename Tp>
using vec = std::vector<Tp>;

template<typename Tp>
using shared_ptr = std::shared_ptr<Tp>;

template<typename Value>
using uset = std::unordered_set<Value>;

#endif /* __TYPES__HH */