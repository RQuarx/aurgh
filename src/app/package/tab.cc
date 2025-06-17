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

#include <filesystem>
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
#include <gtkmm/window.h>
#include <gtkmm/label.h>
#include <glibmm/fileutils.h>
#include <glibmm/refptr.h>

#include "package/client.hh"
#include "package/card.hh"
#include "package/tab.hh"
#include "widget_ptr.hh"
#include "config.hh"
#include "dialog.hh"
#include "logger.hh"
#include "utils.hh"
#include "data.hh"

using pkg::Tab;

using Gtk::ScrolledWindow;
using Gtk::ComboBoxText;
using Gtk::CheckButton;
using Gtk::SearchEntry;
using Gtk::Expander;
using Gtk::Button;
using Gtk::Label;
using Gtk::Box;


Tab::Tab() :
    m_actions (std::make_shared<pkg::Actions>()),
    m_use_dark((*data::config->get_config())["app"]["use-dark-icon"].asBool()),
    m_ui_base ((*data::config->get_config())["paths"]["ui-path"].asString()),
    m_spinner (Gtk::make_managed<Gtk::Spinner>())
{
    data::logger->log(Logger::Debug, "Creating packages tab");

    str icon_name = std::format("pkg_icon_{}.svg",
                                m_use_dark ? "dark" : "light");
    data::package_icon_file = utils::get_ui_file(icon_name, m_ui_base);

    /* Process:
       1. Creating Gtk::Builder from tab.xml.
       2. Setup the whole widget structure + Gtk::Spinner.
       3. Connect the search dispatcher to the function.
    */
    builder_t b;

    try {
        b = Gtk::Builder::create_from_file(
            utils::get_ui_file("tab.xml", m_ui_base));
    } catch (const Glib::Error &e) {
        data::logger->log(
            Logger::Error,
            "Failed to parse .xml file: {}",
#if GTK4
            e.what()
#else
            e.what().raw()
#endif
        );
    }

    setup_widgets(b);
    setup();
    get_installed_pkgs();
    m_search_dispatcher.connect([this]()-> void { on_dispatch_search_ready(); });
}


void
Tab::setup_widgets(const builder_t &p_b) /* p_b -> p_builder */
{
#if GTK4
    m_tab_box            = p_b->get_widget<Box>("tab_main");
    m_result_box         = p_b->get_widget<Box>("result_box");
    m_results_scroll     = p_b->get_widget<ScrolledWindow>("search_results");
    m_search_by_combo    = p_b->get_widget<ComboBoxText>("search_by");
    m_sort_by_combo      = p_b->get_widget<ComboBoxText>("sort_by");
    m_reverse_sort_check = p_b->get_widget<CheckButton>("reverse_sort");
    m_search_entry       = p_b->get_widget<SearchEntry>("search_entry");
    m_no_actions_label   = p_b->get_widget<Label>("no_actions_label");
    m_execute_button     = p_b->get_widget<Button>("execute_actions_button");

    m_action_widgets[Install] = p_b->get_widget<Expander>("actions_install");
    m_action_widgets[Remove]  = p_b->get_widget<Expander>("actions_remove");
    m_action_widgets[Update]  = p_b->get_widget<Expander>("actions_update");

    m_spinner->set_valign(Gtk::Align::CENTER);
    m_result_box->append(*m_spinner);
    append(*m_tab_box);
#else
    p_b->get_widget("tab_main",               m_tab_box);
    p_b->get_widget("result_box",             m_result_box);
    p_b->get_widget("search_results",         m_results_scroll);
    p_b->get_widget("search_by",              m_search_by_combo);
    p_b->get_widget("sort_by",                m_sort_by_combo);
    p_b->get_widget("reverse_sort",           m_reverse_sort_check);
    p_b->get_widget("search_entry",           m_search_entry);
    p_b->get_widget("no_actions_label",       m_no_actions_label);
    p_b->get_widget("execute_actions_button", m_execute_button);

    p_b->get_widget("actions_install", m_action_widgets[Install]);
    p_b->get_widget("actions_remove",  m_action_widgets[Remove]);
    p_b->get_widget("actions_update",  m_action_widgets[Update]);

    m_spinner->set_valign(Gtk::ALIGN_CENTER);
    m_result_box->pack_start(*m_spinner);
    add(*m_tab_box);
    set_visible();
#endif
}


void
Tab::setup()
{
    vec<str>
        search_by_keywords = pkg::Client::get_search_by_keywords(),
        sort_by_keywords   = pkg::Client::get_sort_by_keywords();

    for (const auto &w : search_by_keywords) m_search_by_combo->append(w);
    for (const auto &w : sort_by_keywords)   m_sort_by_combo->append(w);

    json cache = *data::config->get_cache();

    m_search_by_combo->set_active_text(cache["search-by-default"].asString());
    m_sort_by_combo->set_active_text(  cache["sort-by-default"]  .asString());
    m_reverse_sort_check->set_active(  cache["reverse-sort-default"].asBool());

    if (m_search_by_combo->get_active_text() == nullptr) {
        m_search_by_combo->set_active_text("name");
    }
    if (m_sort_by_combo->get_active_text() == nullptr) {
        m_sort_by_combo->set_active_text("NumVotes");
    }

    auto criteria_changed = [this]() -> void {
        json cache = *data::config->get_cache();

        cache["sort-by-default"]      =
            m_sort_by_combo->get_active_text().raw();
        cache["search-by-default"]    =
            m_search_by_combo->get_active_text().raw();
        cache["reverse-sort-default"] =
            m_reverse_sort_check->get_active();

        data::config->save();
        on_search();
    };

    m_reverse_sort_check->signal_toggled() .connect(criteria_changed);
    m_search_by_combo   ->signal_changed() .connect(criteria_changed);
    m_search_entry      ->signal_activate().connect(criteria_changed);
    m_sort_by_combo     ->signal_changed() .connect(criteria_changed);

    m_execute_button->signal_clicked().connect([this](){
        on_execute_button_pressed();
    });
}


void
Tab::on_search()
{
    /* Don't search if there's already a "Search process" running */
    if (m_running) return;

    /* Removes all cards from the results box,
       on GTK 3, the spinner would still occupy a space
       even though it is not visible.
       So the only way to actually "hide" it,
       it to remove it from the container and add it later.
    */
    auto children = m_result_box->get_children();
    for (auto *child : children) {
        if (dynamic_cast<Gtk::Spinner*>(child) == nullptr) {
            m_result_box->remove(*child);
            delete child;
            continue;
        }

#if GTK3
        if (dynamic_cast<Gtk::Spinner*>(child) != nullptr) {
            m_result_box->remove(*child);
        }
#endif
    }

    /* Putting this here means the results box will be cleared
       before the search entry is empty check is done.
       Which means, the results box will be empty
       if the search entry is empty.
    */
    str pkg_name = m_search_entry->get_text();
    if (pkg_name.empty()) return;

    /* Start the spinner */
#if GTK4
    m_result_box->set_valign(Gtk::Align::CENTER);
    m_spinner->set_visible();
#else
    m_result_box->set_valign(Gtk::ALIGN_CENTER);
    m_result_box->pack_start(*m_spinner);
#endif

    m_spinner->start();

    str search_by = m_search_by_combo->get_active_text();

    data::logger->log(Logger::Info,
                      R"(Searching for "{}", by "{}")",
                      pkg_name, search_by);

    /* Search the object */
    std::jthread([this, pkg_name, search_by](){
        m_running = true;

        /* It is unlikely for CURL to fail, so adding attributes will make
           the compiler to be able to add more optimizations regarding this
           try-catch thing.
        */
        try { [[likely]]
            m_search_result = data::pkg_client->search(pkg_name, search_by);
        } catch (const std::exception &e) { [[unlikely]]
            data::logger->log(Logger::Error, "Failed to search: {}", e.what());
            m_search_result = json(Json::arrayValue);
        }

        m_search_dispatcher.emit();
    }).detach();
}


void
Tab::on_dispatch_search_ready()
{
    /* Show a "No packages found." message when no packages were found. */
    if (m_search_result["resultcount"].asInt() < 1) { [[unlikely]]
        data::logger->log(Logger::Debug, "Found no packages.");
        m_spinner->set_visible(false);

        auto no_packages_label =
            std::make_unique<Gtk::Label>("No packages found.");

#if GTK4
        m_result_box->append(*no_packages_label);
#else
        m_result_box->pack_start(*no_packages_label);
        m_result_box->show_all_children();
#endif
        m_running = false;
        return;
    }

    data::logger->log(Logger::Debug,
                      "Found {} packages.",
                      m_search_result["resultcount"].asInt());
    generate_cards();
    m_running = false;
}


void
Tab::generate_cards()
{
    data::logger->log(Logger::Debug, "Starting generating cards.");

    /* Fetch the package informations */
    str  sort_by = m_sort_by_combo->get_active_text();
    bool reverse = m_reverse_sort_check->get_active();
    json pkgs    = m_search_result["results"];
    auto sorted  = utils::sort_json(pkgs, sort_by, reverse);

    /* Stop the spinner */
    m_spinner->stop();

#if GTK4
    m_result_box->set_valign(Gtk::Align::START);
    m_spinner->set_visible(false);
#else
    m_result_box->set_valign(Gtk::ALIGN_START);
    m_result_box->remove(*m_spinner);
#endif

    if (m_card_ui_file.empty()) { [[unlikely]]
        m_card_ui_file = utils::get_ui_file("card.xml", m_ui_base);
    }

    /* Creating cards */
    size_t card_amounts = 0;
    for (const auto &pkg : sorted) {
        builder_t  builder = Gtk::Builder::create_from_file(m_card_ui_file);
        pkg::Card *card    = nullptr;
#if GTK4
        card = Gtk::Builder::get_widget_derived<pkg::Card>(builder,
                                                           "pkg_card",
                                                           pkg);
        m_result_box->append(*card);
#else
        builder->get_widget_derived<pkg::Card>("pkg_card", card, pkg);
        m_result_box->pack_start(*card);
#endif

        card->signal_action_pressed().connect(sigc::mem_fun(
            *this, &Tab::on_action_button_pressed
        ));
        card_amounts++;
    }

    data::logger->log(Logger::Debug,
                      "Finished generating {} cards.",
                      card_amounts);
}


void
Tab::on_action_button_pressed(pkg::Type   p_type,
                              bool        p_action_type,
                              const json &p_pkg)
{
    if (p_action_type) {
        m_actions->at(p_type)->push_back(p_pkg["Name"].asString());
    } else {
        auto it = std::ranges::find(*m_actions->at(p_type),
                                     p_pkg["Name"].asString());

        if (it != m_actions->at(p_type)->end()) {
            m_actions->at(p_type)->erase(it);
        }
    }

    /* Check for unresolved dependencies, currently disabled. */
    if (p_action_type && p_type != Type::Remove) {
        if (has_unresolved_dependencies(p_pkg)) {
            data::logger->log(Logger::Debug,
                              "Package {} has unresolved deps",
                              p_pkg["Name"].asString());
            builder_t builder = Gtk::Builder::create_from_file(
                utils::get_ui_file("dialog.xml", m_ui_base));
            widget_ptr<Dialog> window;
            Dialog            *ptr = nullptr;

#if GTK4
            ptr = Gtk::Builder::get_widget_derived<Dialog>(
                builder,
                "confirmation_window",
                "Unresolved Dependencies",
                std::format("The package {} has some unresolved dependencies, "
                            "do you want to install them?",
                            p_pkg["Name"].asString()),
                m_use_dark
            );
#else
            builder->get_widget_derived<Dialog>(
                "confirmation_window",
                ptr,
                "Unresolved Dependencies",
                std::format("The package {} has some unresolved dependencies, "
                            "do you want to install them?",
                            p_pkg["Name"].asString()),
                m_use_dark
            );
#endif
            window = ptr;

            if (!window->wait_for_response()) {
                data::app->remove_window(*window);
                return;
            }
            data::app->remove_window(*window);
        }
    }

    refresh_actions();
}


void
Tab::refresh_actions()
{
    bool all_empty = true;
    for (auto t : { Install, Remove, Update }) {
        auto  pkgs   = m_actions->at(t);
        auto *action = m_action_widgets.at(t);

        /* If the current action is emtpy, that means
           the current expander action should not be shown,
           and if it the last type (Update) and all_empty is still true,
           then that means all of the actions are empty.
        */
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

        remove_all_child(*dynamic_cast<Gtk::Box *>(action->get_child()));

        for (const auto &action_pkg : *pkgs) {
            auto *link = Gtk::make_managed<Gtk::LinkButton>();
            str   url  = std::format("https://aur.archlinux.org/packages/{}",
                                     action_pkg);

            link->set_tooltip_text(url);
            link->set_label(action_pkg);
            link->set_visible();
            link->set_uri(url);

#if GTK4
            link->set_halign(Gtk::Align::START);
            dynamic_cast<Gtk::Box *>(action->get_child())->append(*link);
#else
            link->set_halign(Gtk::ALIGN_START);
            dynamic_cast<Gtk::Box *>(action->get_child())->pack_start(*link);
#endif

        }

        action->set_visible(true);
#if GTK3
        action->show_all_children();
#endif
    }
}


void
Tab::on_execute_button_pressed()
{
    if (!m_actions->remove->empty()) {
        data::pkg_client->remove(*m_actions->remove);
        m_actions->remove->clear();
    }

    if (!m_actions->install->empty()) {
        data::pkg_client->install(*m_actions->install);
        m_actions->install->clear();
    }

    on_search();
}


auto
Tab::has_unresolved_dependencies(const json &p_pkg) -> bool
{
    json info { data::pkg_client->info(p_pkg["Name"].asString()) };

    vec<json> all_deps;
    for (const auto &d : info["results"][0]["Depends"])     all_deps.push_back(d);
    for (const auto &d : info["results"][0]["MakeDepends"]) all_deps.push_back(d);

    if (all_deps.empty()) return false;

    static umap<str, bool> pkg_exists_cache;

    return std::ranges::any_of(all_deps, [&](const auto &dep_val) {
        str dep_name  = dep_val.asString();

        if (dep_name.contains('>')) return false;
        if (dep_name.contains('<')) return false;
        if (dep_name.contains('=')) return false;

        if (data::pkg_client->find_pkg(dep_name) == nullptr) return true;
        return !data::installed_pkgs->contains(dep_name);
    });
}


void
Tab::get_installed_pkgs()
{
    data::installed_pkgs->clear();

    auto local_pkgs     = data::pkg_client->get_locally_installed_pkgs();
    auto installed_pkgs = data::pkg_client->get_installed_pkgs();

    data::installed_pkgs->reserve(local_pkgs.size() + installed_pkgs.size());

    for (auto *pkg : local_pkgs) {
        data::installed_pkgs->emplace(alpm_pkg_get_name(pkg));
    }
    for (auto *pkg : installed_pkgs) {
        data::installed_pkgs->emplace(alpm_pkg_get_name(pkg));
    }
}


void
Tab::remove_all_child(Gtk::Box &p_container)
{
    auto children = p_container.get_children();
    std::ranges::for_each(children, [&p_container](Gtk::Widget *child){
        p_container.remove(*child);
        delete child;
    });
}