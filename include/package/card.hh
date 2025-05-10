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
#ifndef PACKAGE_CARD_HH__
#define PACKAGE_CARD_HH__

#include <utility>
#include <string>
#include <vector>

#include <gtkmm/frame.h>
#include <json/value.h>

#include "type.hh"


namespace Gtk {
    class Button;
    class Box;
}
class Logger;

using str_pair = std::pair<std::string, std::string>;


namespace pkg {
    /**
     * @class Card
     * @brief A widget that displays information for an AUR package
     * @ingroup Widget
     * @ingroup Container
     * @ingroup Frame
     */
    class Card : public Gtk::Frame
    {
        using slot_type = sigc::slot<bool, GdkEventButton *>;
    public:
        explicit Card(
            Json::Value package,
            const std::vector<str_pair> &installed_aur_packages,
            std::shared_ptr<Actions> &actions,
            const std::shared_ptr<Logger> &logger,
            int32_t spacing = DEFAULT_SPACING
        );

        auto get_action_button() -> Gtk::Button*;

    private:
        static const inline int32_t DEFAULT_SPACING = 5;

        Gtk::Button *m_action_button;

        std::vector<str_pair>      m_installed_package;
        std::shared_ptr<Actions>   m_actions;
        std::shared_ptr<Logger>    m_logger;
        Json::Value                m_package;

        int32_t m_default_spacing;

        void create_action_button();
        void create_info_box(Gtk::Box &box) const;
        void create_package_name(Gtk::Box &box) const;
        void create_popularity_frame(Gtk::Frame &frame) const;

        /**
         * @brief Searches for a package inside installed_aur_package
         * @returns -1 on none, 0 on all, 1 on name.
         */
        auto find_package(const str_pair &package) const -> int8_t;
    };
} /* namespace pkg */

#endif /* package/card.hh */