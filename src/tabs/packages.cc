#include <gtkmm-3.0/gtkmm/frame.h>
#include "tabs/packages.hh"
#include "aur_client.hh"
#include "logger.hh"
#include "utils.hh"


PackageTab::PackageTab(AUR_Client *aur_client, Logger *logger) :
    m_search_results(Gtk::make_managed<Gtk::ScrolledWindow>()),
    m_search_by_combo(Gtk::make_managed<Gtk::ComboBoxText>()),
    m_entry(Gtk::make_managed<Gtk::SearchEntry>()),
    m_aur_client(aur_client),
    m_logger(logger)
{
    m_logger->log(Logger::Debug, "Creating packages tab");
    GtkUtils::set_margin(*this, 5);
    set_orientation(Gtk::ORIENTATION_VERTICAL);

    auto *frame = Gtk::make_managed<Gtk::Frame>();
    frame->add(*create_search_box());
    pack_start(*frame, false, false);

    auto *results_box = Gtk::make_managed<Gtk::Box>();
    m_search_results->set_placement(Gtk::CORNER_TOP_RIGHT);

    Gtk::Label *label = GtkUtils::create_label_markup("<b>Search to view AUR packages</b>");
    label->set_opacity(0.5);
    auto *info_box = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL, 5);
    info_box->pack_start(*label);
    info_box->set_halign(Gtk::ALIGN_CENTER);
    m_search_results->add(*info_box);

    results_box->pack_start(*m_search_results, true, true);
    pack_start(*results_box, true, true);

    show_all_children();
}


auto
PackageTab::create_search_box() -> Gtk::Box*
{
    auto *main_box = Gtk::make_managed<Gtk::Box>();

    auto search_by_keywords = AUR_Client::get_search_by_keywords();
    for (const auto &s : search_by_keywords) {
        m_search_by_combo->append(s);
    }
    m_search_by_combo->set_active_text(search_by_keywords.at(0));
    m_search_by_combo->signal_changed().connect(
        sigc::mem_fun(*this, &PackageTab::on_search)
    );

    m_entry->set_placeholder_text("Search AUR package");
    m_entry->signal_search_changed().connect(
        sigc::mem_fun(*this, &PackageTab::on_search)
    );
    m_entry->set_margin_left(5);

    main_box->pack_start(*m_search_by_combo, false, true);
    main_box->pack_start(*m_entry, true, true);
    GtkUtils::set_margin(*main_box, 5);

    return main_box;
}


void
PackageTab::on_search()
{
    if (m_entry->get_text_length() < 3) return;
    m_search_results->remove();

    std::string package_name = m_entry->get_text();
    std::string search_by = m_search_by_combo->get_active_text();
    m_logger->log(
        Logger::Info,
        "Searching for {}, by {}",
        package_name, search_by
    );

    auto aur_packages     = m_aur_client->search(package_name, search_by);
    auto *frame  = Gtk::make_managed<Gtk::Frame>();
    auto *packages = Gtk::make_managed<Gtk::Box>();
    packages->set_spacing(10);

    if (aur_packages.empty()) return;
    if (aur_packages["type"].asString() == "error") return;
    for (auto package : aur_packages["results"]) {
        auto *name = Gtk::make_managed<Gtk::Button>(package["Name"].asString());

        packages->pack_start(*name);
    }

    frame->add(*packages);
    m_search_results->add(*frame);
    m_search_results->show_all_children();
}