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
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <glibmm/refptr.h>

#include "aur_client.hh"
#include "config.hh"
#include "logger.hh"
#include "card.hh"
#include "tab.hh"

using pkg::Tab;

using Gtk::ScrolledWindow;
using Gtk::ComboBoxText;
using Gtk::CheckButton;
using Gtk::SearchEntry;
using Gtk::Expander;
using Gtk::Button;
using Gtk::Label;
using Gtk::Box;


Tab::Tab(
    const shared_ptr<AUR::Client> &aur_client,
    const shared_ptr<Logger>      &logger,
    const shared_ptr<Config>      &config,
    const shared_ptr<ArgParser>   &arg_parser
) :
    m_aur_client(aur_client),
    m_logger(logger),
    m_config(config),
    m_actions(std::make_shared<pkg::Actions>()),
    m_searching(false),
    m_card_ui_file(utils::get_ui_file("card.xml", arg_parser)),
    m_spinner(Gtk::make_managed<Gtk::Spinner>())
{
    m_logger->log(Logger::Debug, "Creating packages tab");

    m_search_dispatcher.connect([this]() {
        on_dispatch_search_ready();
    });

    Glib::RefPtr<Gtk::Builder> b;

    try {
        b = Gtk::Builder::create_from_file(
            utils::get_ui_file("tab.xml", arg_parser)
        );
    } catch (const Gtk::BuilderError& e) {
        m_logger->log(
            Logger::Error,
            "Failed to parse .xml file: {}",
            e.what()
        );
    }

    setup_widgets(b);
    m_spinner->hide();
    setup();

#if GTK4
    m_result_box->append(*m_spinner);
    append(*m_tab_box);
#else
    m_result_box->pack_start(*m_spinner);
    add(*m_tab_box);
    set_visible();
#endif
}


void
Tab::setup_widgets(const Glib::RefPtr<Gtk::Builder> &b)
{
#if GTK4
    m_tab_box            = b->get_widget<Box>("tab_main");
    m_result_box         = b->get_widget<Box>("result_box");
    m_results_scroll     = b->get_widget<ScrolledWindow>("search_results");
    m_search_by_combo    = b->get_widget<ComboBoxText>("search_by");
    m_sort_by_combo      = b->get_widget<ComboBoxText>("sort_by");
    m_reverse_sort_check = b->get_widget<CheckButton>("reverse_sort");
    m_search_entry       = b->get_widget<SearchEntry>("search_entry");
    m_no_actions_label   = b->get_widget<Label>("no_actions_label");
    m_execute_button     = b->get_widget<Button>("execute_actions_button");

    m_action_widgets[Install] = b->get_widget<Expander>("actions_install");
    m_action_widgets[Remove]  = b->get_widget<Expander>("actions_remove");
    m_action_widgets[Update]  = b->get_widget<Expander>("actions_update");

    m_spinner->set_valign(Gtk::Align::CENTER);
#else
    b->get_widget("tab_main",               m_tab_box);
    b->get_widget("result_box",             m_result_box);
    b->get_widget("search_results",         m_results_scroll);
    b->get_widget("search_by",              m_search_by_combo);
    b->get_widget("sort_by",                m_sort_by_combo);
    b->get_widget("reverse_sort",           m_reverse_sort_check);
    b->get_widget("search_entry",           m_search_entry);
    b->get_widget("no_actions_label",       m_no_actions_label);
    b->get_widget("execute_actions_button", m_execute_button);

    b->get_widget("actions_install", m_action_widgets[Install]);
    b->get_widget("actions_remove",  m_action_widgets[Remove]);
    b->get_widget("actions_update",  m_action_widgets[Update]);

    m_spinner->set_valign(Gtk::ALIGN_CENTER);
#endif
}


void
Tab::setup()
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
        config["sort-by-default"]      =
            m_sort_by_combo->get_active_text().raw();
        config["search-by-default"]    =
            m_search_by_combo->get_active_text().raw();
        config["reverse-sort-default"] =
            m_reverse_sort_check->get_active();

        m_config->save(config);
        on_search();
    }, config);

    m_search_by_combo->signal_changed().connect(criteria_changed);
    m_sort_by_combo->signal_changed().connect(criteria_changed);
    m_reverse_sort_check->signal_toggled().connect(criteria_changed);
    m_search_entry->signal_activate().connect(criteria_changed);

    m_execute_button->signal_clicked().connect([this](){
        if (!on_execute_button_pressed()) {
            exit(EXIT_FAILURE);
        }
    });
}

void
Tab::on_search()
{
    auto children = m_result_box->get_children();

    for (auto *child : children) {
#if GTK4
        if (dynamic_cast<Gtk::Spinner*>(child) == nullptr) {
#endif
            m_result_box->remove(*child);
#if GTK4
        }
#endif
    }

#if GTK4
    m_result_box->set_valign(Gtk::Align::CENTER);
    m_spinner->set_visible();
#else
    m_result_box->set_valign(Gtk::ALIGN_CENTER);
    m_result_box->pack_start(*m_spinner);
    m_spinner->set_visible();
#endif

    m_spinner->start();

    std::string pkg_name  = m_search_entry->get_text();
    std::string search_by = m_search_by_combo->get_active_text();

    m_logger->log(
        Logger::Info, "Searching for: {}, by {}", pkg_name, search_by
    );

    std::thread([this, pkg_name, search_by](){
        m_searching = true;
        m_running = true;

        try {
            m_search_result = m_aur_client->search(pkg_name, search_by);
        } catch (const std::exception &e) {
            m_logger->log(Logger::Error, "Failed to search: {}", e.what());
            m_search_result = Json::Value(Json::arrayValue);
        }

        m_searching = false;
        m_search_dispatcher.emit();
    }).detach();
}


void
Tab::on_dispatch_search_ready()
{
    std::string  sort_by        = m_sort_by_combo->get_active_text();
    bool         reverse        = m_reverse_sort_check->get_active();
    Json::Value  pkgs           = m_search_result["results"];
    Json::Value  sorted         = utils::sort_json(pkgs, sort_by, reverse);
    auto         local_pkgs     = m_aur_client->get_installed_pkgs();
    str_pair_vec local_pkgs_str;
    local_pkgs_str.reserve(local_pkgs.size());

    for (auto *pkg : local_pkgs) {
        local_pkgs_str.emplace_back(
            alpm_pkg_get_name(pkg), alpm_pkg_get_version(pkg)
        );
    }

    m_spinner->stop();

#if GTK4
    m_result_box->set_valign(Gtk::Align::START);
#else
    m_result_box->set_valign(Gtk::ALIGN_START);
    m_result_box->remove(*m_spinner);
#endif
    m_spinner->set_visible(false);

    for (const auto &pkg : sorted) {
        auto *card = Gtk::make_managed<pkg::Card>(
            pkg,
            m_card_ui_file,
            local_pkgs_str,
            m_logger,
            m_actions
        );

        card->get_action_button()->signal_clicked().connect(sigc::mem_fun(
            *this, &Tab::on_action_button_pressed
        ));

#if GTK4
        m_result_box->append(*card);
#else
        m_result_box->pack_start(*card);
#endif
    }

#if GTK3
    m_result_box->show_all_children();
#endif
    m_running = false;
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
                m_no_actions_label->set_visible(true);
            }
            action->set_visible(false);
            continue;
        }

        if (all_empty) {
            all_empty = false;
            m_no_actions_label->set_visible(false);
        }

        auto *box     = dynamic_cast<Gtk::Box*>(action->get_child());
        auto children = box->get_children();
        for (auto *child : children) {
            box->remove(*child);
        }

        for (const auto &pkg : *pkgs) {
            auto *link = Gtk::make_managed<Gtk::LinkButton>();
            std::string url = std::format("https://aur.archlinux.org/packages/{}", pkg);

            link->set_label(pkg);
            link->set_uri(url);
            link->set_tooltip_text(url);

#if GTK4
            link->set_halign(Gtk::Align::START);
            link->set_visible();
            box->append(*link);
#else
            link->set_halign(Gtk::ALIGN_START);
            box->pack_start(*link);
#endif

        }

        action->set_visible(true);
#if GTK3
        action->show_all_children();
#endif
    }
}

void
#if GTK4
Tab::on_action_type_opened(pkg::Type type)
#else
Tab::on_action_type_opened(GdkEventButton * /*button_event*/, pkg::Type type)
#endif
{
    auto *action_widget = m_action_widgets.at(type);

    if (!m_action_widgets[type]->get_expanded()) {
        auto pkgs     = m_actions->at(type);
        auto *box     = dynamic_cast<Gtk::Box*>(action_widget->get_child());
        auto children = box->get_children();
        for (auto *child : children) {
            box->remove(*child);
        }

        for (const auto &pkg : *pkgs) {
            auto *link      = Gtk::make_managed<Gtk::LinkButton>();
            std::string url = std::format(
                "https://aur.archlinux.org/packages/{}", pkg
            );

            link->set_tooltip_text(url);
            link->set_label(pkg);
            link->set_uri(url);

#if GTK4
            link->set_halign(Gtk::Align::START);
            box->append(*link);
#else
            link->set_halign(Gtk::ALIGN_START);
            box->pack_start(*link);
#endif
        }
    }
}


auto
Tab::on_execute_button_pressed() -> bool
{
    auto removed = m_actions->remove;

    m_aur_client->remove(*removed);

    on_search();
    for (auto t : { pkg::Update, pkg::Install, pkg::Remove }) {
#if GTK4
        on_action_type_opened(t);
#else
        on_action_type_opened(nullptr, t);
#endif
    }

    return true;
}