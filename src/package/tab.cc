/**
 * @file package/tab.cc
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

#include <thread>

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/searchentry.h>
#include <gtkmm/linkbutton.h>
#include <gtkmm/expander.h>
#include <gtkmm/spinner.h>
#include <gtkmm/label.h>
#include <gtkmm/image.h>
#include <glibmm/main.h>

#include "package/card.hh"
#include "package/tab.hh"
#include "aur_client.hh"
#include "logger.hh"
#include "utils.hh"

using pkg::Tab;


Tab::Tab(AUR::Client *aur_client, Logger *logger) :
    m_search_results(Gtk::make_managed<Gtk::ScrolledWindow>()),
    m_search_by(Gtk::make_managed<Gtk::ComboBoxText>()),
    m_sort_by(Gtk::make_managed<Gtk::ComboBoxText>()),
    m_reverse_sort(Gtk::make_managed<Gtk::CheckButton>()),
    m_entry(Gtk::make_managed<Gtk::SearchEntry>()),
    m_spinner(Gtk::make_managed<Gtk::Spinner>()),
    m_quote_label(Gtk::make_managed<Gtk::Label>()),
    m_result_box(Gtk::make_managed<Gtk::Box>()),
    m_actions_widget(Gtk::make_managed<Gtk::Expander>()),
    m_aur_client(aur_client),
    m_logger(logger)
{
    m_package_dispatcher.connect([this](){
        on_dispatch_search_ready();
    });

    m_quote_dispatcher.connect([this](){
        if (m_quote_label) {
            m_quote_label->set_markup(std::format("<i>{}</i>", m_quote));
        }
    });

    m_logger->log(Logger::Debug, "Creating packages tab");

    auto *action_box  = Gtk::make_managed<Gtk::Box>();
    auto *frame     = Gtk::make_managed<Gtk::Frame>();
    auto *results_box = Gtk::make_managed<Gtk::Box>();
    auto *label     = Gtk::make_managed<Gtk::Label>();

    action_box->set_orientation(Gtk::ORIENTATION_VERTICAL);
    action_box->set_margin_left(10);

    for (const auto &t : { pkg::Install, pkg::Remove, pkg::Update }) {
        m_actions_view.at(t) = Gtk::make_managed<Gtk::Expander>();
        m_actions_view.at(t)->set_label(std::format("{}", t));

        m_actions_view.at(t)->signal_button_press_event().connect(sigc::bind(
            sigc::mem_fun(this, &Tab::on_action_type_opened), t
        ));

        action_box->pack_start(*m_actions_view.at(t));
    }

    GtkUtils::set_margin(*this, m_default_spacing);
    set_orientation(Gtk::ORIENTATION_VERTICAL);

    label->set_markup("<b>Search to view AUR packages</b>");
    label->set_opacity(0.5);

    m_result_box->pack_start(*label);
    m_result_box->set_orientation(Gtk::ORIENTATION_VERTICAL);
    m_result_box->set_spacing(m_default_spacing);
    m_result_box->set_halign(Gtk::ALIGN_CENTER);
    m_result_box->set_hexpand();

    m_search_results->add(*m_result_box);
    m_search_results->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    m_search_results->set_placement(Gtk::CORNER_TOP_RIGHT);

    m_actions_widget->set_label("Actions");
    m_actions_widget->set_visible(false);
    m_actions_widget->add(*action_box);

    results_box->pack_start(*m_search_results, true, true);
    results_box->pack_start(*m_actions_widget, false, false);
    results_box->set_margin_top(m_default_spacing);

    frame->add(*create_search_box());

    pack_start(*frame, false, false);
    pack_start(*results_box, true, true);
    show_all_children();
}


auto
Tab::create_search_box() -> Gtk::Box*
{
    auto *main_box          = Gtk::make_managed<Gtk::Box>();
    auto *search_by_frame = Gtk::make_managed<Gtk::Frame>();
    auto *sort_by_frame   = Gtk::make_managed<Gtk::Frame>();
    auto *sort_by_box       = Gtk::make_managed<Gtk::Box>();

    auto *entry_box   = Gtk::make_managed<Gtk::Box>();
    auto *misc_box    = Gtk::make_managed<Gtk::Box>();
    auto *menu        = Gtk::make_managed<Gtk::Box>();
    auto *menu_icon = Gtk::make_managed<Gtk::Image>();

    auto search_by_keywords = AUR::Client::get_search_by_keywords();
    auto sort_by_keywords   = AUR::Client::get_sort_by_keywords();

    for (const auto &w : search_by_keywords) m_search_by->append(w);
    for (const auto &w : sort_by_keywords) m_sort_by->append(w);

    m_search_by->signal_changed().connect([this](){ on_search(); });
    m_search_by->set_active_text(search_by_keywords.at(0));
    GtkUtils::set_margin(*m_search_by, m_default_spacing);

    search_by_frame->set_label("Search by");
    search_by_frame->add(*m_search_by);
    GtkUtils::set_margin(*search_by_frame, 0, m_default_spacing);

    m_sort_by->signal_changed().connect([this](){ on_search(); });
    m_sort_by->set_active_text(sort_by_keywords.at(0));
    GtkUtils::set_margin(*m_sort_by, m_default_spacing);

    m_reverse_sort->signal_clicked().connect([this](){ on_search(); });
    m_reverse_sort->set_tooltip_text("Reverse sort");
    m_reverse_sort->set_halign(Gtk::ALIGN_CENTER);
    m_reverse_sort->set_valign(Gtk::ALIGN_CENTER);
    GtkUtils::set_margin(*m_reverse_sort, { 0, m_default_spacing, 0, 0 });

    sort_by_box->pack_start(*m_sort_by);
    sort_by_box->pack_start(*m_reverse_sort);
    sort_by_box->set_spacing(m_default_spacing);
    sort_by_box->set_valign(Gtk::ALIGN_START);

    sort_by_frame->set_label("Sort by");
    sort_by_frame->add(*sort_by_box);
    GtkUtils::set_margin(*sort_by_frame, 0, m_default_spacing);

    m_entry->signal_search_changed().connect([this](){ on_search(); });
    m_entry->set_placeholder_text("Search for AUR packages");
    m_entry->set_valign(Gtk::ALIGN_CENTER);
    m_entry->set_vexpand(false);

    m_quote_label->set_markup("<i>...</i>");
    m_quote_label->set_halign(Gtk::ALIGN_START);

    menu_icon->set_from_icon_name("open-menu", Gtk::ICON_SIZE_MENU);

    menu->add(*menu_icon);
    menu->set_halign(Gtk::ALIGN_END);

    misc_box->pack_start(*m_quote_label);
    misc_box->pack_start(*menu);
    entry_box->pack_start(*misc_box);
    entry_box->pack_start(*m_entry);
    entry_box->set_orientation(Gtk::ORIENTATION_VERTICAL);

    main_box->pack_start(*search_by_frame, false, true);
    main_box->pack_start(*sort_by_frame, false, true);
    main_box->pack_start(*entry_box, true, true);

    main_box->set_spacing(m_default_spacing);
    GtkUtils::set_margin(*main_box, m_default_spacing);

    std::thread([this](){
        m_quote = Utils::get_quote(75);
        m_quote_dispatcher.emit();
    }).detach();

    return main_box;
}


void
Tab::on_search()
{
    if (m_entry->get_text_length() < 3) return;

    std::string package_name = m_entry->get_text();
    std::string search_by = m_search_by->get_active_text();
    m_logger->log(
        Logger::Info,
        "Searching for {}, by {}",
        package_name, search_by
    );

    auto children = m_result_box->get_children();
    for (auto *w : children) m_result_box->remove(*w);

    m_spinner->start();
    m_result_box->set_halign(Gtk::ALIGN_CENTER);
    m_result_box->pack_start(*m_spinner);
    m_result_box->show_all_children();

    std::thread([this, package_name, search_by](){
        m_aur_packages.clear();
        m_aur_packages = m_aur_client->search(package_name, search_by);

        m_spinner->stop();
        if (m_spinner->get_parent() != nullptr) {
            m_result_box->remove(*m_spinner);
        }

        m_result_box->set_halign(Gtk::ALIGN_START);
        m_package_dispatcher.emit();
    }).detach();
}


void
Tab::on_dispatch_search_ready()
{
    for (size_t i = 0; i < m_package_queue.size(); i++) {
        m_package_queue.pop();
    }

    if (m_aur_packages["type"].asString() == "error") {
        auto *label = Gtk::make_managed<Gtk::Label>();

        label->set_markup("<b>An error has occured</b>");

        m_result_box->pack_start(*label);
        m_result_box->set_halign(Gtk::ALIGN_CENTER);
    }
    else if (m_aur_packages["resultcount"].asInt() == 0) {
        auto *label = Gtk::make_managed<Gtk::Label>();

        label->set_markup("<b>No results found</b>");

        m_result_box->pack_start(*label);
        m_result_box->set_halign(Gtk::ALIGN_CENTER);
    }
    else {
        for (const auto &package : sort_packages(m_aur_packages)) {
            m_package_queue.push(package);
        }

        process_next_package(get_installed_aur_packages());
        m_result_box->set_halign(Gtk::ALIGN_START);
    }

    m_result_box->show_all_children();
}


void
Tab::process_next_package(
    const std::vector<std::pair<std::string, std::string>> &installed_packages)
{
    if (m_package_queue.empty()) return;

    auto package = m_package_queue.front();
    m_package_queue.pop();

    auto *card = Gtk::make_managed<Card>(
        package, installed_packages, &m_actions, m_logger);

    card->get_action_button()->signal_clicked().connect(sigc::mem_fun(
        this, &Tab::on_action_button_pressed
    ));

    m_result_box->pack_start(*card, true, true);
    m_result_box->show_all_children();

    Glib::signal_timeout().connect_once([this, installed_packages](){
        process_next_package(installed_packages);
    }, 100);
}


auto
Tab::get_installed_aur_packages(
    ) -> std::vector<std::pair<std::string, std::string>>
{
    std::vector<std::pair<std::string, std::string>> installed_packages;
    std::istringstream iss(Utils::run_command("pacman -Qm", 512)->first);
    std::string line;

    while (std::getline(iss, line)) {
        auto package = Str::splitp(line, line.find(' '));
        installed_packages.emplace_back(
            Str::trim(package.at(0)),
            Str::trim(package.at(1))
        );
    }

    return installed_packages;
}


auto
Tab::sort_packages(Json::Value packages) -> std::vector<Json::Value>
{
    std::vector<Json::Value> package;
    package.reserve(packages["results"].size());

    for (const auto &p : packages["results"]) {
        package.push_back(p);
    }

    std::string sort_by = m_sort_by->get_active_text();

    std::ranges::sort(package,
        [this, &sort_by](const Json::Value &a, const Json::Value &b){
        if (a[sort_by].isInt()) {

            if (m_reverse_sort->get_active()) {
                return a[sort_by].asInt64() < b[sort_by].asInt64();
            }
            return a[sort_by].asInt64() > b[sort_by].asInt64();
        }

        if (m_reverse_sort->get_active()) {
            return a[sort_by].asString() < b[sort_by].asString();
        }
        return a[sort_by].asString() > b[sort_by].asString();
    });

    return package;
}


auto
Tab::on_action_type_opened(
    GdkEventButton * /*button_event*/, pkg::Type type) -> bool
{
    if (!m_actions_view.at(type)->get_expanded()) {
        std::vector<std::string> *pkgs = m_actions.at(type);

        auto *packages = Gtk::make_managed<Gtk::Box>();

        packages->set_orientation(Gtk::ORIENTATION_VERTICAL);
        packages->set_margin_left(10);

        for (const auto &pkg : *pkgs) {
            auto *url_button = Gtk::make_managed<Gtk::LinkButton>();

            std::string url = std::format(
                "https://aur.archlinux.org/packages/{}", pkg
            );

            url_button->set_halign(Gtk::ALIGN_START);
            url_button->set_tooltip_text(url);
            url_button->set_label(pkg);
            url_button->set_uri(url);

            packages->pack_start(*url_button);
        }

        m_actions_view.at(type)->remove();
        m_actions_view.at(type)->add(*packages);
        m_actions_view.at(type)->show_all_children();
    }
    return true;
}


void
Tab::on_action_button_pressed()
{
    bool all_empty = true;
    for (auto t : { pkg::Install, pkg::Remove, pkg::Update }) {
        auto *const pkgs = m_actions.at(t);
        auto *action = m_actions_view.at(t);

        if (pkgs->empty()) {
            if (t == pkg::Update && all_empty) {
                m_actions_widget->set_visible(false);
            }

            action->set_visible(false);
            action->remove();
            continue;
        }

        if (all_empty) {
            all_empty = false;
            m_actions_widget->set_visible(true);
        }

        auto *packages = Gtk::make_managed<Gtk::Box>();

        packages->set_orientation(Gtk::ORIENTATION_VERTICAL);
        packages->set_margin_left(10);

        for (const auto &pkg : *pkgs) {
            auto *url_button = Gtk::make_managed<Gtk::LinkButton>();

            std::string url = std::format(
                "https://aur.archlinux.org/packages/{}", pkg
            );

            url_button->set_halign(Gtk::ALIGN_START);
            url_button->set_tooltip_text(url);
            url_button->set_label(pkg);
            url_button->set_uri(url);

            packages->pack_start(*url_button);
        }

        action->remove();
        action->add(*packages);
        action->set_visible(true);
        action->show_all_children();
    }
}