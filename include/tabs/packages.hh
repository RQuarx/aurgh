#pragma once
#ifndef TABS_PACKAGES_HH__
#define TABS_PACKAGES_HH__

#include <gtkmm-3.0/gtkmm/box.h>
#include <json/value.h>

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
    auto create_package_card(const Json::Value &package) -> Gtk::Frame*;

    void on_search();
};

#endif /* tabs/packages.hh */