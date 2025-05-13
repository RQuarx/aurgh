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
#ifndef PACKAGE_TAB_HH__
#define PACKAGE_TAB_HH__

#include <atomic>
#include <memory>

#include <glibmm/dispatcher.h>
#include <json/value.h>
#include <gtkmm/box.h>

#include "package/type.hh"

namespace AUR { class Client; }
namespace Gtk {
    class ScrolledWindow;
    class ComboBoxText;
    class CheckButton;
    class SearchEntry;
    class Expander;
    class Builder;
    class Spinner;
    class Label;
}
class ArgParser;
class Config;
class Logger;


namespace pkg {
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
        using str_pair_vec = std::vector<std::pair<std::string, std::string>>;
    public:
        Tab(
            const shared_ptr<AUR::Client> &aur_client,
            const shared_ptr<Logger>      &logger,
            const shared_ptr<Config>      &config,
            const shared_ptr<ArgParser>   &arg_parser
        );

    private:
        shared_ptr<AUR::Client> m_aur_client;
        shared_ptr<Logger>      m_logger;
        shared_ptr<Config>      m_config;
        shared_ptr<Actions>     m_actions;

        std::atomic<bool>        m_searching;
        std::atomic<bool>        m_running;
        Glib::Dispatcher         m_search_dispatcher;
        std::vector<Json::Value> m_package_queue;
        Json::Value              m_search_result;

        std::string m_card_ui_file;

        /* Widgets */

        Gtk::Box *m_tab_box{};
        Gtk::Box *m_result_box{};

        Gtk::ScrolledWindow *m_results_scroll{};

        Gtk::ComboBoxText *m_search_by_combo{};
        Gtk::ComboBoxText *m_sort_by_combo{};
        Gtk::CheckButton  *m_reverse_sort_check{};
        Gtk::SearchEntry  *m_search_entry{};

        Gtk::Expander                       *m_actions_expander{};
        std::map<pkg::Type, Gtk::Expander *> m_action_widgets;

        Gtk::Spinner *m_spinner{};

    protected:
        auto setup() -> bool;
        auto sort_packages(const Json::Value &pkgs) -> std::vector<Json::Value>;

#if GTKMM_MAJOR_VERSION == 4
        void on_action_type_opened(pkg::Type type);
#elif
        void on_action_type_opened(GdkEventButton *button_event, pkg::Type type);
#endif
        void process_next_package(const str_pair_vec &installed);
        void on_action_button_pressed();
        void on_dispatch_search_ready();
        void on_search();

        static auto get_installed_pkgs() -> str_pair_vec;
    };
} /* namespace pkg */

#endif /* package/tab.hh */