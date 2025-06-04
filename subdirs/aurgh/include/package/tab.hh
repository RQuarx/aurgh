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
#ifndef __PACKAGE__TAB_HH__
#define __PACKAGE__TAB_HH__

#include <atomic>
#include <memory>

#include <glibmm/dispatcher.h>
#include <json/value.h>
#include <gtkmm/box.h>

#include "package/type.hh"

namespace Gtk {
    class ScrolledWindow;
    class ComboBoxText;
    class CheckButton;
    class SearchEntry;
    class Expander;
    class Builder;
    class Spinner;
    class Button;
    class Label;
} /* namespace Gtk */
class ArgParser;
class Config;
class Logger;


namespace pkg {
    class Client;

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
        Tab(
            const shared_ptr<pkg::Client> &aur_client,
            const shared_ptr<Logger>      &logger,
            const shared_ptr<Config>      &config,
            const shared_ptr<ArgParser>   &arg_parser
        );

    protected:
        /**
         * @brief Initializes and connects all widgets from the Gtk::Builder UI.
         * @param b Reference to the Gtk::Builder used to build the UI.
         */
        void setup_widgets(const Glib::RefPtr<Gtk::Builder> &b);


        /**
         * @brief Populates combo boxes, sets initial states, and connects signal handlers.
         */
        void setup();


        /**
         * @brief Triggered when a search is requested by the user.
         */
        void on_search();


        /**
         * @brief Triggered when the action buttons is pressed.
         */
        void on_action_button_pressed(const Json::Value &pkg);


        /**
         * @brief Handles the event when search results are ready (via dispatcher).
         */
        void on_dispatch_search_ready();


        /**
         * @brief Triggered when the execute action button is pressed.
         */
        auto on_execute_button_pressed() -> bool;


        /**
         * @brief Resolves the full UI file path from a filename.
         * @param file_name Name of the UI file.
         * @return Full file path to the UI resource.
         */
        auto get_ui_file(const std::string &file_name) -> std::string;


        /**
         * @brief Checks if a package @p pkg has unresolved dependency.
         * @param pkg The package to check for.
         */
        auto has_unresolved_dependencies(const Json::Value &pkg) -> bool;


        /**
         * @brief Fills the m_card_data.installed_pkgs with the installed pkgs.
         */
        void get_installed_pkgs();

#if GTKMM_MAJOR_VERSION == 4
        /**
         * @brief Opens the appropriate action for a package (GTK4 version).
         * @param type The type of package action selected.
         */
        void on_action_type_opened(pkg::Type type);
#else
        /**
         * @brief Opens the appropriate action for a package (GTK3 version).
         * @param button_event Pointer to the button press event.
         * @param type         The type of package action selected.
         */
        void on_action_type_opened(GdkEventButton *button_event, pkg::Type type);
#endif

    private:
        shared_ptr<Client>    m_aur_client;
        shared_ptr<Logger>    m_logger;
        shared_ptr<ArgParser> m_arg_parser;
        shared_ptr<Config>    m_config;
        shared_ptr<Actions>   m_actions;

        pkg_uset m_installed_pkgs;
        CardData m_card_data;

        std::atomic<bool>        m_searching;
        std::atomic<bool>        m_running;
        Glib::Dispatcher         m_search_dispatcher;
        std::vector<Json::Value> m_package_queue;
        Json::Value              m_search_result;
        std::string              m_card_ui_file;

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
        Gtk::Spinner *m_spinner{};
    };
} /* namespace pkg */

#endif /* __PACKAGE__TAB_HH__ */