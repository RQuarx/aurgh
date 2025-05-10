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

#include <fstream>
#include <queue>

#include <glibmm/dispatcher.h>
#include <json/value.h>
#include <gtkmm/box.h>

#include "card.hh"
#include "utils.hh"

namespace Gtk {
    class ScrolledWindow;
    class ComboBoxText;
    class CheckButton;
    class SearchEntry;
    class Expander;
    class Spinner;
    class Frame;
}
namespace AUR { class Client; }
class Logger;


namespace pkg {
    /**
    * @class PackageTab
    * @brief A tab for AUR package management
    * @ingroup Widget
    * @ingroup Container
    * @ingroup Box
    */
    class Tab : public Gtk::Box
    {
    public:
        explicit Tab(
            const std::shared_ptr<AUR::Client> &aur_client,
            const std::shared_ptr<Logger> &logger
        );

    private:
        const int32_t m_default_spacing = 5;

        Gtk::ScrolledWindow *m_search_results;
        Gtk::ComboBoxText   *m_search_by;
        Gtk::ComboBoxText   *m_sort_by;
        Gtk::CheckButton    *m_reverse_sort;
        Gtk::SearchEntry    *m_entry;
        Gtk::Spinner        *m_spinner;
        Gtk::Label          *m_quote_label;
        Gtk::Box            *m_result_box;
        Gtk::Expander       *m_actions_widget;

        std::unordered_map<pkg::Type, Gtk::Expander*> m_actions_view{
            { pkg::Install, nullptr },
            { pkg::Remove, nullptr },
            { pkg::Update, nullptr }
        };

        Glib::Dispatcher m_package_dispatcher;
        Glib::Dispatcher m_quote_dispatcher;

        std::shared_ptr<AUR::Client> m_aur_client;
        std::shared_ptr<Logger>      m_logger;

        std::queue<Json::Value>  m_package_queue;
        Json::Value              m_aur_packages;
        std::shared_ptr<Actions> m_actions;

        std::string m_quote;
        std::string m_cache_path;

    protected:
        auto save_cache() -> bool;

        auto create_search_box() -> Gtk::Box*;

        void on_search();
        void on_dispatch_search_ready();
        void on_download_clicked();
        void process_next_package(
            const std::vector<std::pair<std::string, std::string>> &installed_packages);

        void on_action_button_pressed();
        void on_card_download_clicked(
            const std::string &pkg_name,
            const std::string &pkg_version,
            int8_t find_result,
            Gtk::Button *&button
        );
        auto on_action_type_opened(
            GdkEventButton *button_event, pkg::Type type) -> bool;

        auto sort_packages(Json::Value packages) -> std::vector<Json::Value>;
        static auto get_installed_aur_packages(
            ) -> std::vector<std::pair<std::string, std::string>>;
    };
} /* namespace pkg */

#endif /* package/tab.hh */