#include <print>
#include <gtkmm-3.0/gtkmm/frame.h>
#include "tabs/packages.hh"
#include "aur_client.hh"


PackageTab::PackageTab(AUR_Client *aur_client) :
    m_aur_client(aur_client), m_search_results(Gtk::make_managed<Gtk::ScrolledWindow>())
{
    set_orientation(Gtk::ORIENTATION_VERTICAL);
    set_spacing(5);

    pack_start(*create_search_box(), false, false);

    auto *results_box = Gtk::make_managed<Gtk::Box>();
    results_box->pack_start(*m_search_results, true, true);
    pack_start(*results_box, true, true);

    show_all_children();
}


auto
PackageTab::create_search_box() -> Gtk::Box*
{
    auto *main_box = Gtk::make_managed<Gtk::Box>();

    m_search_by_combo = Gtk::make_managed<Gtk::ComboBoxText>();
    for (const auto &s : m_search_by_keywords) {
        m_search_by_combo->append(s);
    }
    m_search_by_combo->set_active_text(m_search_by_keywords.at(0));

    m_search_bar = Gtk::make_managed<Gtk::SearchBar>();
    m_entry = Gtk::make_managed<Gtk::SearchEntry>();
    m_search_bar->connect_entry(*m_entry);

    m_entry->signal_search_changed().connect(
        sigc::mem_fun(*this, &PackageTab::on_search)
    );

    m_entry->set_halign(Gtk::ALIGN_CENTER);

    main_box->pack_start(*m_search_by_combo, false, true);
    main_box->pack_start(*m_entry);

    return main_box;
}


void
PackageTab::on_search()
{
    if (m_entry->get_text_length() < 3) return;

    auto aur_packages =
        m_aur_client->search(m_entry->get_text(), m_search_by_combo->get_active_text());
    update_search_result(aur_packages);
}


void
PackageTab::update_search_result(const Json::Value &aur_packages)
{
    if (aur_packages.empty()) return;
    if (aur_packages["type"].asString() == "error") return;

    m_search_results->remove();

    auto *frame = Gtk::make_managed<Gtk::Frame>();
    // frame->set_shadow_type(Gtk::SHADOW_IN);

    auto *packages = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL, 10);
    packages->set_spacing(10);

    for (auto package : aur_packages["results"]) {
        auto *name = Gtk::make_managed<Gtk::Button>(package["Name"].asString());

        packages->pack_start(*name);
    }

    frame->add(*packages);
    m_search_results->add(*frame);
    m_search_results->show_all_children();
}