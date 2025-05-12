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

#include <thread>

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/searchentry.h>
#include <gtkmm/linkbutton.h>
#include <gtkmm/expander.h>
#include <gtkmm/spinner.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/builder.h>

#include "package/card.hh"
#include "package/tab.hh"
#include "aur_client.hh"
#include "config.hh"
#include "logger.hh"

using pkg::Tab;


Tab::Tab(
    const shared_ptr<AUR::Client> &aur_client,
    const shared_ptr<Logger> &logger,
    const shared_ptr<Config> &config,
    const shared_ptr<ArgParser> &arg_parser
) :
    m_aur_client(aur_client),
    m_logger(logger),
    m_config(config),
    m_actions(std::make_shared<pkg::Actions>()),
    m_card_ui_file(utils::get_ui_file("card.xml", arg_parser)),
    m_spinner(Gtk::make_managed<Gtk::Spinner>())
{
    m_logger->log(Logger::Debug, "Creating packages tab");

    m_search_dispatcher.connect([this]() {
        on_dispatch_search_ready();
    });

    auto builder = Gtk::Builder::create_from_file(
        utils::get_ui_file("tab.xml", arg_parser)
    );

    builder->get_widget("tab_main",        m_tab_box);
    builder->get_widget("result_box",      m_result_box);
    builder->get_widget("search_results",  m_results_scroll);
    builder->get_widget("search_by",       m_search_by_combo);
    builder->get_widget("sort_by",         m_sort_by_combo);
    builder->get_widget("reverse_sort",    m_reverse_sort_check);
    builder->get_widget("search_entry",    m_search_entry);
    builder->get_widget("actions_widget",  m_actions_expander);

    builder->get_widget("actions_install", m_action_widgets[Install]);
    builder->get_widget("actions_remove",  m_action_widgets[Remove]);
    builder->get_widget("actions_update",  m_action_widgets[Update]);

    if (!setup()) {
        exit(EXIT_FAILURE);
    }

    m_result_box->show_all_children();

    add(*m_tab_box);
    show_all_children();
}



auto
Tab::setup() -> bool
{
    auto search_by_keywords = AUR::Client::get_search_by_keywords();
    auto sort_by_keywords   = AUR::Client::get_sort_by_keywords();

    for (const auto &w : search_by_keywords) m_search_by_combo->append(w);
    for (const auto &w : sort_by_keywords)   m_sort_by_combo->append(w);

    auto opt = m_config->load(true);
    Json::Value config;

    if (opt == std::nullopt) {
        config["sort-by-default"]      = sort_by_keywords.at(0);
        config["search-by-default"]    = search_by_keywords.at(0);
        config["reverse-sort-default"] = false;
    } else { config = *opt; }

    m_search_by_combo->set_active_text(config["search-by-default"].asString());
    m_sort_by_combo->set_active_text(config["sort-by-default"].asString());
    m_reverse_sort_check->set_active(config["reverse-sort-default"].asBool());

    auto criteria_changed = sigc::bind([this](Json::Value config){
        config["sort-by-default"]      = m_sort_by_combo->get_active_text().raw();
        config["search-by-default"]    = m_search_by_combo->get_active_text().raw();
        config["reverse-sort-default"] = m_reverse_sort_check->get_active();

        m_config->save(config);
        on_search();
    }, config);

    m_search_by_combo->signal_changed().connect(criteria_changed);
    m_sort_by_combo->signal_changed().connect(criteria_changed);
    m_reverse_sort_check->signal_clicked().connect(criteria_changed);
    m_search_entry->signal_activate().connect(criteria_changed);
    return true;
}

void
Tab::on_search()
{
    std::string pkg_name  = m_search_entry->get_text();
    std::string search_by = m_search_by_combo->get_active_text();

    m_logger->log(
        Logger::Info, "Searching for: {}, by {}", pkg_name, search_by
    );

    m_search_result = m_aur_client->search(pkg_name, search_by);

    std::jthread([this](){
        m_search_dispatcher.emit();
    }).detach();
}



auto
Tab::sort_packages(const Json::Value &packages) -> std::vector<Json::Value>
{
    std::vector<Json::Value> package;
    package.reserve(packages.size());

    for (const auto &p : packages) package.push_back(p);

    std::string sort_by = m_sort_by_combo->get_active_text();

    std::ranges::sort(package,
        [this, &sort_by](const Json::Value &a, const Json::Value &b){
        if (a[sort_by].isInt()) {
            if (m_reverse_sort_check->get_active()) {
                return a[sort_by].asInt64() > b[sort_by].asInt64();
            }
            return a[sort_by].asInt64() < b[sort_by].asInt64();
        }

        if (m_reverse_sort_check->get_active()) {
            return a[sort_by].asString() > b[sort_by].asString();
        }
        return a[sort_by].asString() < b[sort_by].asString();
    });

    return package;
}


auto
Tab::get_installed_pkgs(
    ) -> std::vector<std::pair<std::string, std::string>>
{
    std::vector<std::pair<std::string, std::string>> installed_packages;
    std::istringstream iss(utils::run_command("pacman -Qm", 512)->first);
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


void
Tab::on_dispatch_search_ready()
{
    m_package_queue.clear();

    for (auto *child : m_result_box->get_children()) {
        m_result_box->remove(*child);
    }

    if (m_search_result["type"].asString() == "error") {
        auto *label = Gtk::make_managed<Gtk::Label>();
        label->set_markup("<b>An error has occurred</b>");
        m_result_box->pack_start(*label);
        m_result_box->set_halign(Gtk::ALIGN_CENTER);
    } else if (m_search_result["resultcount"].asInt() == 0) {
        auto *label = Gtk::make_managed<Gtk::Label>();
        label->set_markup("<b>No results found</b>");
        m_result_box->pack_start(*label);
        m_result_box->set_halign(Gtk::ALIGN_CENTER);
    } else {
        for (const auto &pkg : sort_packages(m_search_result["results"])) {
            m_package_queue.push_back(pkg);
        }

        m_result_box->set_halign(Gtk::ALIGN_START);
        process_next_package(get_installed_pkgs());
    }

    m_result_box->show_all_children();
}


void
Tab::process_next_package(
    const std::vector<std::pair<std::string, std::string>> &installed)
{
    if (m_package_queue.empty()) return;

    auto pkg = m_package_queue.back();
    m_package_queue.pop_back();

    auto *card = Gtk::make_managed<Card>(
        pkg, m_card_ui_file, installed, m_logger, m_actions
    );

    card->get_action_button()->signal_clicked().connect(
        sigc::mem_fun(*this, &Tab::on_action_button_pressed)
    );

    m_result_box->pack_start(*card, true, true);
    m_result_box->show_all_children();

    Glib::signal_idle().connect_once([this, installed]() {
        process_next_package(installed);
    });
}

void
Tab::on_action_button_pressed()
{
    bool all_empty = true;

    for (auto t : { Install, Remove, Update }) {
        auto pkgs = m_actions->at(t);
        auto *action = m_action_widgets.at(t);

        if (pkgs->empty()) {
            if (t == Update && all_empty) {
                m_actions_expander->set_visible(false);
            }
            action->set_visible(false);
            action->remove();
            continue;
        }

        if (all_empty) {
            all_empty = false;
            m_actions_expander->set_visible(true);
        }

        auto *box = Gtk::make_managed<Gtk::Box>();
        box->set_orientation(Gtk::ORIENTATION_VERTICAL);
        box->set_margin_left(10);

        for (const auto &pkg : *pkgs) {
            auto *link = Gtk::make_managed<Gtk::LinkButton>();
            std::string url = std::format("https://aur.archlinux.org/packages/{}", pkg);

            link->set_label(pkg);
            link->set_uri(url);
            link->set_tooltip_text(url);
            link->set_halign(Gtk::ALIGN_START);

            box->pack_start(*link);
        }

        action->remove();
        action->add(*box);
        action->set_visible(true);
        action->show_all_children();
    }
}

void
Tab::on_action_type_opened(GdkEventButton* /*ev*/, pkg::Type type) {
    if (!m_action_widgets[type]->get_expanded()) {
        auto pkgs = m_actions->at(type);
        auto *box = Gtk::make_managed<Gtk::Box>();

        box->set_orientation(Gtk::ORIENTATION_VERTICAL);
        box->set_margin_left(10);

        for (const auto &pkg : *pkgs) {
            auto *link = Gtk::make_managed<Gtk::LinkButton>();
            std::string url = std::format("https://aur.archlinux.org/packages/{}", pkg);

            link->set_label(pkg);
            link->set_uri(url);
            link->set_tooltip_text(url);
            link->set_halign(Gtk::ALIGN_START);

            box->pack_start(*link);
        }

        m_action_widgets[type]->remove();
        m_action_widgets[type]->add(*box);
        m_action_widgets[type]->show_all_children();
    }
}