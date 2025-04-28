/**
 * @file package/tab.hh
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

#include <queue>

#include <glibmm/dispatcher.h>
#include <json/value.h>
#include <gtkmm/box.h>

#include "package/card.hh"
#include "utils.hh"

namespace Gtk {
    class ScrolledWindow;
    class ComboBoxText;
    class CheckButton;
    class SearchEntry;
    class Spinner;
    class Frame;
}
class AUR_Client;
class Logger;


/**
 @class PackageTab (Gtk::Box)
 @brief A tab for AUR package management
 */
class PackageTab : public Gtk::Box
{
public:
    explicit PackageTab(AUR_Client *aur_client, Logger *logger);

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

    Glib::Dispatcher m_package_dispatcher;
    Glib::Dispatcher m_quote_dispatcher;

    AUR_Client *m_aur_client;
    Logger     *m_logger;

    std::queue<Json::Value> m_package_queue;
    Json::Value             m_aur_packages;

    std::string m_quote;

protected:
    auto create_search_box() -> Gtk::Box*;

    void on_search();
    void on_dispatch_search_ready();
    void on_download_clicked();
    void process_next_package(
        const std::vector<Utils::str_pair> &installed_packages);

    static auto get_installed_aur_packages() -> std::vector<Utils::str_pair>;
    auto sort_packages(Json::Value packages) -> std::vector<Json::Value>;
};

#endif /* package/tab.hh */