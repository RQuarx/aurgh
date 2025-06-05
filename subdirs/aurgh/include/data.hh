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
#ifndef __DATA_HH__
#define __DATA_HH__

#include <memory>
#include "package/client.hh"
#include "arg_parser.hh"
#include "config.hh"
#include "logger.hh"


namespace data {
    using pkg::Client;

    inline std::shared_ptr<ArgParser> arg_parser;
    inline std::shared_ptr<Logger>    logger;
    inline std::shared_ptr<Config>    config;
    inline std::shared_ptr<Client>    pkg_client;
} /* namespace data */


#endif /* __DATA_HH__ */