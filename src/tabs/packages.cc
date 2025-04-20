/**
 * @file tabs/packages.cc
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

#include <gtkmm-3.0/gtkmm/scrolledwindow.h>
#include <gtkmm-3.0/gtkmm/comboboxtext.h>
#include <gtkmm-3.0/gtkmm/searchentry.h>
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
    results_box->set_margin_top(5);
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


auto
PackageTab::create_package_card(
    const Json::Value &package,
    const std::vector<Utils::str_pair> &installed_packages
) -> Gtk::Frame*
{
    auto *frame          = Gtk::make_managed<Gtk::Frame>();
    auto *card             = Gtk::make_managed<Gtk::Box>();

    auto *basic_info       = Gtk::make_managed<Gtk::Box>();
    auto *full_info        = Gtk::make_managed<Gtk::Box>();
    auto *version        = Gtk::make_managed<Gtk::Label>();
    auto *popularity     = Gtk::make_managed<Gtk::Label>();
    auto *desc           = Gtk::make_managed<Gtk::Label>();

    auto *action_button = Gtk::make_managed<Gtk::Button>();

    std::string markup = "<b>{}</b>";
    if (package["OutOfDate"] != Json::Value::null) {
        markup = Utils::format(markup, "<span foreground=\"red\">{}</span>");
    }

    std::string package_name = Str::trim(package["Name"].asString());
    std::string package_ver  = Str::trim(package["Version"].asString());

    Gtk::Box *name = GtkUtils::create_label_icon(
        "package-x-generic-symbolic",
        Utils::format(markup, package_name),
        Gtk::ICON_SIZE_MENU
    );

    markup = "<sub>{}</sub>";
    version->set_markup(Utils::format(markup, package_ver));
    popularity->set_markup(
        Utils::format(markup,
            std::format("[+{} ~{}]",
                package["NumVotes"].asInt(),
                std::roundf(package["Popularity"].asFloat() * 100) / 100
            )
        )
    );

    basic_info->set_spacing(5);
    basic_info->set_halign(Gtk::ALIGN_START);
    basic_info->pack_start(*name, false, false);
    basic_info->pack_start(*version, false, false);
    basic_info->pack_start(*popularity, false, false);

    desc->set_label(package["Description"].asString());
    desc->set_halign(Gtk::ALIGN_START);
    desc->set_line_wrap(true);

    action_button->set_image_from_icon_name(
        Utils::find(installed_packages, { package_name, package_ver })
        ? "edit-delete"
        : "document-download"
    );
    action_button->set_valign(Gtk::ALIGN_CENTER);
    action_button->set_halign(Gtk::ALIGN_END);

    full_info->set_orientation(Gtk::ORIENTATION_VERTICAL);
    full_info->pack_start(*basic_info, true, true);
    full_info->pack_start(*desc, true, true);
    full_info->set_margin_left(5);

    card->set_spacing(5);
    card->pack_start(*full_info, true, true);
    card->pack_end(*action_button, true, true);
    GtkUtils::set_margin(*card, 5);

    frame->add(*card);
    GtkUtils::set_margin(*frame, 5);

    return frame;
}


void
PackageTab::on_search()
{
    if (m_entry->get_text_length() < 3) return;

    std::string package_name = m_entry->get_text();
    std::string search_by = m_search_by_combo->get_active_text();
    m_logger->log(
        Logger::Info,
        "Searching for {}, by {}",
        package_name, search_by
    );

    auto aur_packages = m_aur_client->search(package_name, search_by);

    if (aur_packages.empty()) return;
    if (aur_packages["type"].asString() == "error") return;

    auto *frame  = Gtk::make_managed<Gtk::Frame>();
    auto *packages = Gtk::make_managed<Gtk::Box>();

    for (const auto& package : aur_packages["results"]) {
        /* The cards will look something like
            ┌──────────────────────────────────────┐
            │ Name  Version [Votes Popularity]     │
            │ Description                        + │
            └──────────────────────────────────────┘
        */

        packages->pack_start(*create_package_card(package, get_installed_aur_packages()), false, false);
    }

    packages->set_orientation(Gtk::ORIENTATION_VERTICAL);

    frame->set_label("Packages");
    frame->add(*packages);
    m_search_results->remove();
    m_search_results->add(*frame);
    m_search_results->show_all_children();
}


auto
PackageTab::get_installed_aur_packages(
    ) -> std::vector<Utils::str_pair>
{
    std::vector<Utils::str_pair> installed_packages(10);
    std::istringstream iss(Utils::run_command("pacman -Qm", 512)->first);
    std::string line;

    while (std::getline(iss, line)) {
        auto package = Str::split(line, line.find(' '));
        installed_packages.emplace_back(Str::trim(package.at(0)), Str::trim(package.at(1)));
    }

    return installed_packages;
}