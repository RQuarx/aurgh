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
#include <chrono>
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
#include <glibmm/fileutils.h>
#include <glibmm/refptr.h>

#include "package/client.hh"
#include "package/card.hh"
#include "package/tab.hh"
#include "config.hh"
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
    m_actions(std::make_shared<pkg::Actions>()),
    m_card_ui_file(get_ui_file("card.xml")),
    m_spinner(Gtk::make_managed<Gtk::Spinner>())
{
    data::logger->log(Logger::Debug, "Creating packages tab");

    /* Prepare to create cards */
    bool use_dark =
        (*data::config->get_config())["app"]["use-dark-icon"].asBool();

    str icon_name = std::format("pkg_icon_{}.svg",
                                use_dark ? "dark" : "light");
    data::package_icon_file = get_ui_file(icon_name);

    /* Process:
       1. Creating Gtk::Builder from tab.xml.
       2. Setup the whole widget structure + Gtk::Spinner.
       3. Connect the search dispatcher to the function.
    */
    builder_t b;

    try {
        b = Gtk::Builder::create_from_file(get_ui_file("tab.xml"));
    } catch (const Glib::FileError &e) {
        data::logger->log(
            Logger::Error,
            "Failed to parse .xml file: {}",
            e.what()
        );
    }

    setup_widgets(b);
    m_spinner->set_visible(false);
    setup();

    #if GTK4
        m_result_box->append(*m_spinner);
        append(*m_tab_box);
    #else
        m_result_box->pack_start(*m_spinner);
        add(*m_tab_box);
        set_visible();
    #endif

    get_installed_pkgs();

    m_search_dispatcher.connect([this]() {
        on_dispatch_search_ready();
    });
}


void
Tab::setup_widgets(const builder_t &b)
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
    vec<str>
        search_by_keywords = pkg::Client::get_search_by_keywords(),
        sort_by_keywords   = pkg::Client::get_sort_by_keywords();

    for (const auto &w : search_by_keywords) m_search_by_combo->append(w);
    for (const auto &w : sort_by_keywords)   m_sort_by_combo->append(w);

    auto cache = data::config->get_cache();

    m_search_by_combo->set_active_text((*cache)["search-by-default"].asString());
    m_sort_by_combo->set_active_text((*cache)["sort-by-default"].asString());
    m_reverse_sort_check->set_active((*cache)["reverse-sort-default"].asBool());

    if (m_search_by_combo->get_active_text() == nullptr) {
        m_search_by_combo->set_active_text("name");
    }
    if (m_sort_by_combo->get_active_text() == nullptr) {
        m_sort_by_combo->set_active_text("NumVotes");
    }

    auto criteria_changed = sigc::bind(
    [this](const shared_ptr<json> &cache)
    {
        (*cache)["sort-by-default"]      =
            m_sort_by_combo->get_active_text().raw();
        (*cache)["search-by-default"]    =
            m_search_by_combo->get_active_text().raw();
        (*cache)["reverse-sort-default"] =
            m_reverse_sort_check->get_active();

        data::config->save();
        on_search();
    }, cache);

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
    /* Don't search if there's already a "Search process" running */
    if (m_running) return;

    /* Removes all cards from the results box */
    auto children = m_result_box->get_children();
    for (auto *child : children) {
        #if GTK4
            if (dynamic_cast<Gtk::Spinner*>(child) == nullptr) {
                m_result_box->remove(*child);
            }
        #else
            m_result_box->remove(*child);
        #endif
    }

    /* Putting this here means the results box will clear
       when the search entry is empty
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
    m_spinner->set_visible(true);
#endif

    m_spinner->start();

    str search_by = m_search_by_combo->get_active_text();

    data::logger->log(
        Logger::Info, "Searching for: {}, by {}", pkg_name, search_by
    );

    /* Search the object */
    std::jthread([this, pkg_name, search_by](){
        m_running = true;

        m_package_queue.clear();
        m_package_queue.resize(0);
        m_search_result.clear();
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

        auto *no_packages_label =
            Gtk::make_managed<Gtk::Label>("No packages found.");

#if GTK4
        m_result_box->append(*no_packages_label);
#else
        m_result_box->pack_start(*no_packages_label);
        m_result_box->show_all_children();
#endif
        m_running = false;
        return;
    }

    data::logger->log(
        Logger::Debug,
        "Found {} packages.",
        m_search_result["resultcount"].asInt()
    );

    /* Fetch the package informations */
    str  sort_by = m_sort_by_combo->get_active_text();
    bool reverse = m_reverse_sort_check->get_active();
    json pkgs    = m_search_result["results"];
    auto sorted  = utils::sort_json(pkgs, sort_by, reverse);

    /* Stop the spinner */
    m_spinner->stop();

#if GTK4
    m_result_box->set_valign(Gtk::Align::START);
#else
    m_result_box->set_valign(Gtk::ALIGN_START);
    m_result_box->remove(*m_spinner);
#endif
    m_spinner->set_visible(false);

    data::logger->log(Logger::Debug, "Starting generating cards.");

    /* Creating cards */
    for (const auto &pkg : sorted) {
        builder_t  builder = Gtk::Builder::create_from_file(m_card_ui_file);
        pkg::Card *card    = nullptr;
#if GTK4
        card = Gtk::Builder::get_widget_derived<pkg::Card>(builder,
                                                           "pkg_card",
                                                           pkg,
                                                           m_actions);
#else
        builder->get_widget_derived<pkg::Card>("pkg_card",
                                                card,
                                                pkg,
                                                m_actions);
#endif

        card->signal_action_pressed().connect(sigc::mem_fun(
            *this, &Tab::on_action_button_pressed
        ));

#if GTK4
        m_result_box->append(*card);
#else
        m_result_box->pack_start(*card);
        card->show_all_children();
#endif
    }

    data::logger->log(Logger::Debug, "Finished generating cards.");

    m_running = false;
}


void
Tab::on_action_button_pressed(pkg::Type type, bool action_type, const json &pkg)
{
    /* Check for unresolved dependencies, currently disabled. */
    if (action_type && type != Type::Remove) {
        if (has_unresolved_dependencies(pkg)) {
            data::logger->log(Logger::Debug,
                              "Package {} has unresolved deps",
                              pkg["Name"].asString());
        }
    }

    bool all_empty = true;

    for (auto t : { Install, Remove, Update }) {
        auto  pkgs   = m_actions->at(t);
        auto *action = m_action_widgets.at(t);

        /* If the current action empty */
        if (pkgs->empty()) {
            /* If it is the last action and all_empty is still true
               Make the "No actions." label visible.
            */
            if (t == Update && all_empty) {
                m_no_actions_label->set_visible(true);
            }

            /* else, just make the action label not visible */
            action->set_visible(false);
            continue;
        }

        if (all_empty) {
            all_empty = false;
            m_no_actions_label->set_visible(false);
        }

        remove_all_child(*dynamic_cast<Gtk::Box*>(action->get_child()));

        for (const auto &action_pkg : *pkgs) {
            auto *link = Gtk::make_managed<Gtk::LinkButton>();
            str url = std::format(
                "https://aur.archlinux.org/packages/{}", action_pkg
            );

            link->set_label(action_pkg);
            link->set_uri(url);
            link->set_tooltip_text(url);

#if GTK4
            link->set_halign(Gtk::Align::START);
            link->set_visible();
            dynamic_cast<Gtk::Box*>
            (action->get_child())->append(*link);
#else
            link->set_halign(Gtk::ALIGN_START);
            dynamic_cast<Gtk::Box*>
            (action->get_child())->pack_start(*link);
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
        auto pkgs = m_actions->at(type);
        remove_all_child(*dynamic_cast<Gtk::Box*>(action_widget->get_child()));


        for (const auto &pkg : *pkgs) {
            auto *link      = Gtk::make_managed<Gtk::LinkButton>();
            str url = std::format(
                "https://aur.archlinux.org/packages/{}", pkg
            );

            link->set_tooltip_text(url);
            link->set_label(pkg);
            link->set_uri(url);

#if GTK4
            link->set_halign(Gtk::Align::START);
            dynamic_cast<Gtk::Box*>
            (action_widget->get_child())->append(*link);
#else
            link->set_halign(Gtk::ALIGN_START);
            dynamic_cast<Gtk::Box*>
            (action_widget->get_child())->pack_start(*link);
#endif
        }
    }

    get_installed_pkgs();
}


auto
Tab::on_execute_button_pressed() -> bool
{
    if (!m_actions->remove->empty()) {
        data::pkg_client->remove(*m_actions->remove);
        m_actions->remove->clear();
    }

    if (!m_actions->install->empty()) {
        data::pkg_client->install(*m_actions->install);
        m_actions->install->clear();
    }

    for (auto t : { pkg::Update, pkg::Install, pkg::Remove }) {
#if GTK4
        on_action_type_opened(t);
#else
        on_action_type_opened(nullptr, t);
#endif
    }
    on_search();
    std::ranges::for_each(m_result_box->get_children(),
    [](Gtk::Widget *c)
    {
        auto *card = dynamic_cast<pkg::Card*>(c);
        if (card != nullptr) {
            card->refresh();
        }
    });

    return true;
}


auto
Tab::get_ui_file(const std::filesystem::path &file_name) -> str
{
    namespace fs = std::filesystem;

    auto valid_file = [](const fs::path &file_path) -> bool {
        return fs::exists(file_path)
            && fs::is_regular_file(file_path);
    };

    if (valid_file(file_name)) return file_name;

    str
        gtk_version = std::format("gtk{}", GTKMM_MAJOR_VERSION),
        base_path   =
            (*data::config->get_config())["paths"]["ui-path"].asString();

    std::array<fs::path, 8> candidates = {
        fs::path(base_path) / file_name,
        fs::path(base_path) / "ui" / file_name,
        fs::path(base_path) / "ui" / gtk_version / file_name,
        fs::path("ui") / file_name,
        fs::path("ui") / gtk_version / file_name,
        fs::path(gtk_version) / file_name,
        fs::path("/usr/share") / APP_NAME / "ui" / gtk_version / file_name,
        fs::path("/usr/share") / APP_NAME / "ui" / file_name
    };

    for (const fs::path &p : candidates) {
        if (valid_file(p)) return fs::canonical(p);
    }
    return "";
}


auto
Tab::has_unresolved_dependencies(const json &pkg) -> bool
{
    json info { data::pkg_client->info(pkg["Name"].asString()) };

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

        return !data::installed_pkgs->contains(dep_name) ||
                data::pkg_client->find_pkg(dep_name) == nullptr;
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
Tab::remove_all_child(Gtk::Box &container)
{
    auto children = container.get_children();
    std::ranges::for_each(children, [&container](Gtk::Widget *child){
        container.remove(*child);
    });
}