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
    class LinkButton;
    class Button;
    class Label;
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
    public:
        explicit Card(
            Json::Value package,
            const std::string &ui_file,
            const std::vector<str_pair> &installed_aur_packages,
            const std::shared_ptr<Logger> &logger,
            std::shared_ptr<Actions> &actions,
            int32_t spacing = DEFAULT_SPACING
        );

        auto get_action_button() -> Gtk::Button*&;

        void refresh();

    private:
        static const inline int32_t DEFAULT_SPACING = 5;

        std::vector<str_pair>      m_installed_package;
        std::shared_ptr<Actions>   m_actions;
        std::shared_ptr<Logger>    m_logger;
        Json::Value                m_package;

        Gtk::Box *m_card = nullptr;

        Gtk::Button *m_action_button = nullptr;

        Gtk::Label *m_version_label = nullptr;
        Gtk::Label *m_desc_label    = nullptr;

        Gtk::Label      *m_name_label = nullptr;
        Gtk::LinkButton *m_name_link  = nullptr;

        Gtk::Label *m_popularity_label = nullptr;
        Gtk::Label *m_votes_label      = nullptr;

        int32_t m_default_spacing;
        bool    m_button_dimmed;

    protected:
        auto setup() -> bool;

        void on_button_clicked(pkg::Type result, const std::string &pkg_name);

        /**
         * @brief Searches for a package inside installed_aur_package
         * @returns -1 on none, 0 on all, 1 on name.
         */
        auto find_package(const str_pair &package) const -> int8_t;
    };
} /* namespace pkg */

#endif /* package/card.hh */