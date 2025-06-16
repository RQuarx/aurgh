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
#ifndef __PACKAGE__CARD_HH__
#define __PACKAGE__CARD_HH__

#include <sigc++/signal.h>
#include <gtkmm/frame.h>
#include <json/value.h>

#include "types.hh"
#include "type.hh"


namespace Gtk {
    class LinkButton;
    class Button;
    class Image;
    class Label;
    class Box;
}


namespace pkg {
    static const float INACTIVE_OPACITY = 0.5F;
    static const float ACTIVE_OPACITY   = 1.0F;


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
        explicit Card(BaseObjectType      *cobject,
                      const builder_t     &builder,
                      json                 pkg,
                      shared_ptr<Actions> &actions);

        auto signal_action_pressed(
            ) -> sigc::signal<void (pkg::Type, bool, const json &)>;

        void refresh();

    private:
        shared_ptr<Actions>      m_actions;
        json                     m_package;

        Gtk::Box *m_card = nullptr;

        Gtk::Button *m_action_button = nullptr;

        Gtk::Label *m_version_label = nullptr;
        Gtk::Label *m_desc_label    = nullptr;

        Gtk::Label      *m_name_label      = nullptr;
        Gtk::LinkButton *m_name_link       = nullptr;
        Gtk::Label      *m_outofdate_label = nullptr;

        Gtk::Label *m_popularity_label = nullptr;
        Gtk::Label *m_votes_label      = nullptr;

        Gtk::Image *m_package_icon = nullptr;

        bool m_button_dimmed;

        sigc::signal<void (pkg::Type, bool, const json &)> m_signal;

    protected:
        auto setup() -> bool;

        void on_button_clicked(pkg::Type result, const str &pkg_name);

        /**
         * @brief Searches for a package inside installed_aur_package
         * @returns -1 on none, 0 on all, 1 on name.
         */
        static auto find_package(const str_pair &package) -> int8_t;
    };
} /* namespace pkg */

#endif /* __PACKAGE__CARD_HH__ */