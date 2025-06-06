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
#ifndef __TYPES_HH__
#define __TYPES_HH__

#include <unordered_map>
#include <json/value.h>


using json     = Json::Value;
using str      = std::string;
using str_view = std::string_view;

template<typename Key, typename Tp>
using umap = std::unordered_map<Key, Tp>;

template<typename Tp>
using vec = std::vector<Tp>;

template<typename Tp>
using shared_ptr = std::shared_ptr<Tp>;

#endif /* __TYPES_HH__ */