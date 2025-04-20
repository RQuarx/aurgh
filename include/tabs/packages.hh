/**
 * @file tabs/packages.hh
 *
 * This file is part of AURGH
 *
 * AURGH is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * AURGH is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#ifndef TABS_PACKAGES_HH__
#define TABS_PACKAGES_HH__

#include <gtkmm-3.0/gtkmm/box.h>
#include <json/value.h>
#include "utils.hh"

namespace Gtk {
    class ScrolledWindow;
    class ComboBoxText;
    class SearchEntry;
    class Frame;
}
class AUR_Client;
class Logger;


class PackageTab : public Gtk::Box
{
public:
    explicit PackageTab(AUR_Client *aur_client, Logger *logger);

private:
    Gtk::ScrolledWindow *m_search_results;
    Gtk::ComboBoxText   *m_search_by_combo;
    Gtk::SearchEntry    *m_entry;

    AUR_Client *m_aur_client;
    Logger     *m_logger;

protected:
    auto create_search_box() -> Gtk::Box*;
    auto create_package_card(
        const Json::Value &package,
        const std::vector<Utils::str_pair> &installed_packages
    ) -> Gtk::Frame*;

    void on_search();
    void on_download_clicked();


    static auto get_installed_aur_packages() -> std::vector<Utils::str_pair>;
};

#endif /* tabs/packages.hh */