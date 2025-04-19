#pragma once
#ifndef TABS_PACKAGES_HH__
#define TABS_PACKAGES_HH__

#include <gtkmm-3.0/gtkmm/scrolledwindow.h>
#include <gtkmm-3.0/gtkmm/comboboxtext.h>
#include <gtkmm-3.0/gtkmm/searchentry.h>
#include <gtkmm-3.0/gtkmm/searchbar.h>
#include <gtkmm-3.0/gtkmm/box.h>
#include <glibmm-2.68/glibmm/timer.h>
#include <json/value.h>

class AUR_Client;

static const int32_t SEARCH_WAIT_MS = 500;


class PackageTab : public Gtk::Box
{
public:
    explicit PackageTab(AUR_Client *aur_client);
private:
    AUR_Client *m_aur_client;

    std::array<std::string, 14> m_search_by_keywords = {{
        "name", "name-desc", "depends", "checkdepends",
        "optdepends", "makedepends", "maintainer", "submitter",
        "provides", "conflicts", "replaces", "keywords",
        "groups", "comaintainers"
    }};
    size_t m_search_by = 0;

    Gtk::ComboBoxText *m_search_by_combo;
    Gtk::SearchBar *m_search_bar;
    Gtk::SearchEntry *m_entry;

    Gtk::ScrolledWindow *m_search_results;

protected:
    auto create_search_box() -> Gtk::Box*;

    void on_search();
    void update_search_result(const Json::Value &aur_packages);
};

#endif /* tabs/packages.hh */