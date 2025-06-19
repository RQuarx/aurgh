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
#ifndef __PACKAGE__CARD__HH
#define __PACKAGE__CARD__HH

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
    static const double INACTIVE_OPACITY = 0.5F;
    static const double ACTIVE_OPACITY   = 1.0F;


    /**
     * @class Card
     * @brief A widget that displays information for an AUR package
     * @ingroup Widget
     * @ingroup Container
     * @ingroup Frame
     */
    class Card : public Gtk::Frame
    {
        using action_func = void (pkg::Type, bool, const json &);
    public:
        explicit Card(BaseObjectType  *p_cobject,
                      const builder_t &p_builder,
                      json             p_pkg);


        [[nodiscard]]
        auto signal_action_pressed() -> sigc::signal<action_func>;

    private:
        json      m_package;
        pkg::Type m_type;

        Gtk::Box *m_card = nullptr;

        Gtk::LinkButton *m_name_link = nullptr;

        Gtk::Button *m_action_button = nullptr;
        Gtk::Image  *m_package_icon  = nullptr;

        Gtk::Label
            *m_version_label    = nullptr,
            *m_desc_label       = nullptr,
            *m_name_label       = nullptr,
            *m_outofdate_label  = nullptr,
            *m_popularity_label = nullptr,
            *m_votes_label      = nullptr;

        sigc::signal<action_func> m_signal;
        bool                      m_button_dimmed;

    protected:
        void setup();

        void on_button_clicked();

        /**
         * @brief Searches for a package inside installed_aur_package
         * @returns -1 on none, 0 on all, 1 on name.
         */
        static auto find_package(const str_pair &p_pkg) -> i8;
    };
} /* namespace pkg */

#endif /* __PACKAGE__CARD__HH */