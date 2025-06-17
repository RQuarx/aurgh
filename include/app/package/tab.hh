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
#ifndef __PACKAGE__TAB__HH
#define __PACKAGE__TAB__HH

#include <atomic>
#include <memory>

#include <glibmm/dispatcher.h>
#include <json/value.h>
#include <gtkmm/box.h>

#include "package/type.hh"
#include "types.hh"


namespace Gtk {
    class ScrolledWindow;
    class ComboBoxText;
    class CheckButton;
    class SearchEntry;
    class Container;
    class Expander;
    class Builder;
    class Spinner;
    class Button;
    class Label;
} /* namespace Gtk */


namespace pkg {
    class Client;
    class Card;

    using std::shared_ptr;

    /**
     * @class Tab
     * @brief A tab for AUR package management
     * @ingroup Widget
     * @ingroup Container
     * @ingroup Box
     */
    class Tab : public Gtk::Box
    {
    public:
        Tab();

    protected:
        void setup_widgets           (const builder_t &p_builder);

        void on_action_button_pressed(pkg::Type   p_type,
                                      bool        p_action_type,
                                      const json &p_pkg);

        void on_dispatch_search_ready();
        void generate_cards();
        void on_search();
        void setup();
        void refresh_actions();

        /**
         * @brief Opens the appropriate action for a package.
         * @param p_type The type of package action selected.
         */
#if GTK4
        void on_action_type_opened(pkg::Type p_type);
#else

        void on_action_type_opened(GdkEventButton *, pkg::Type p_type);
#endif


        /**
         * @brief Triggered when the execute action button is pressed.
         */
        void on_execute_button_pressed();


        /**
         * @brief Checks if a package @p pkg has unresolved dependency.
         * @param p_pkg The package to check for.
         */
        static auto has_unresolved_dependencies(const json &p_pkg) -> bool;


        static void
        /** @brief Removes all children inside of a Gtk::Box @p p_container . */
            remove_all_child(Gtk::Box &p_container),

        /** @brief Fetches installed packages on the system. */
            get_installed_pkgs();

    private:
        static inline str m_card_ui_file;

        std::atomic<bool> m_running;
        Glib::Dispatcher  m_search_dispatcher;

        shared_ptr<Actions> m_actions;
        bool                m_use_dark;

        vec<json>         m_package_queue;
        json              m_search_result;

        str m_ui_base;

        Gtk::Spinner *m_spinner;

        vec<std::unique_ptr<pkg::Card>> m_cards;

        Gtk::Box *m_tab_box{};
        Gtk::Box *m_result_box{};

        Gtk::ScrolledWindow *m_results_scroll{};

        Gtk::ComboBoxText *m_search_by_combo{};
        Gtk::ComboBoxText *m_sort_by_combo{};
        Gtk::CheckButton  *m_reverse_sort_check{};
        Gtk::SearchEntry  *m_search_entry{};

        Gtk::Label                          *m_no_actions_label{};
        std::map<pkg::Type, Gtk::Expander *> m_action_widgets;

        Gtk::Button  *m_execute_button{};
    };
} /* namespace pkg */

#endif /* __PACKAGE__TAB_HH__ */