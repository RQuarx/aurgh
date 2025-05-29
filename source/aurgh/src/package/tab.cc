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
#include <regex>
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

#include "arg_parser.hh"
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
    m_arg_parser(arg_parser),
    m_config(config),
    m_actions(std::make_shared<pkg::Actions>()),
    m_card_data({
        .card_builder_file = get_ui_file("card.xml"),
        .installed_pkgs    = std::make_shared<str_pair_vec>(),
        .actions           = m_actions,
        .logger            = m_logger
    }),
    m_searching(false),
    m_card_ui_file(get_ui_file("card.xml")),
    m_spinner(Gtk::make_managed<Gtk::Spinner>())
{
    m_logger->log(Logger::Debug, "Creating packages tab");

    builder_t b;

    try {
        b = Gtk::Builder::create_from_file(get_ui_file("tab.xml"));
    } catch (const Glib::FileError &e) {
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
    auto search_by_keywords = AUR::Client::get_search_by_keywords();
    auto sort_by_keywords   = AUR::Client::get_sort_by_keywords();

    for (const auto &w : search_by_keywords) m_search_by_combo->append(w);
    for (const auto &w : sort_by_keywords)   m_sort_by_combo->append(w);

    auto cache = m_config->get_cache();

    m_search_by_combo->set_active_text((*cache)["search-by-default"].asString());
    m_sort_by_combo->set_active_text((*cache)["sort-by-default"].asString());
    m_reverse_sort_check->set_active((*cache)["reverse-sort-default"].asBool());

    auto criteria_changed = sigc::bind(
    [this](const std::shared_ptr<Json::Value> &cache)
    {
        (*cache)["sort-by-default"]      =
            m_sort_by_combo->get_active_text().raw();
        (*cache)["search-by-default"]    =
            m_search_by_combo->get_active_text().raw();
        (*cache)["reverse-sort-default"] =
            m_reverse_sort_check->get_active();

        m_config->save();
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
    m_logger->log(
        Logger::Debug,
        "Found {} packages.",
        m_search_result["resultcount"].asInt()
    );

    std::string  sort_by        = m_sort_by_combo->get_active_text();
    bool         reverse        = m_reverse_sort_check->get_active();
    Json::Value  pkgs           = m_search_result["results"];
    Json::Value  sorted         = utils::sort_json(pkgs, sort_by, reverse);

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
            m_card_data
        );

        card->get_action_button()->signal_clicked().connect(sigc::bind(
            sigc::mem_fun(*this, &Tab::on_action_button_pressed), pkg
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
Tab::on_action_button_pressed(const Json::Value &pkg)
{
    // m_logger->log(Logger::Debug, "Start");
    // if (has_unresolved_dependencies(pkg)) {
    //     m_logger->log(Logger::Debug, "End");
    // }
    // m_logger->log(Logger::Debug, "End");

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
            std::string url = std::format(
                "https://aur.archlinux.org/packages/{}", pkg
            );

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

    get_installed_pkgs();
}


auto
Tab::on_execute_button_pressed() -> bool
{
    if (!m_actions->remove->empty()) {
        m_aur_client->remove(*m_actions->remove);
        m_actions->remove->clear();
    }

    if (!m_actions->install->empty()) {
        m_aur_client->install(*m_actions->install);
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
    [](Gtk::Widget* c)
    {
        auto *card = dynamic_cast<pkg::Card*>(c);
        if (card != nullptr) {
            card->refresh();
        }
    });

    return true;
}


auto
Tab::get_ui_file(const std::string &file_name) -> std::string
{
    namespace fs = std::filesystem;

    auto valid_file = [](const std::string &file){
        return fs::exists(file)
            && fs::is_regular_file(file)
            && fs::path(file).extension() == ".xml";
    };

    std::string gtk_version = std::to_string(GTKMM_MAJOR_VERSION);
    std::string ui_path     = m_arg_parser->get_option("ui");

    if (!ui_path.empty()) {
        ui_path.append(std::format("/gtk{}/{}", gtk_version, file_name));

        ui_path = std::filesystem::absolute(ui_path);

        if (valid_file(ui_path)) return ui_path;
    }

    std::string path = std::format("/ui/gtk{}/{}", gtk_version, file_name);

    if (valid_file(file_name)) return file_name;
    if (valid_file(path)) return path;
    return (*m_config->get_config())["default-ui-path"].asString() + path;
}


auto
Tab::has_unresolved_dependencies(const Json::Value &pkg) -> bool
{
    auto info = m_aur_client->info(pkg["Name"].asString());

    auto installed_pkgs       = m_aur_client->get_installed_pkgs();
    auto local_installed_pkgs = m_aur_client->get_locally_installed_pkgs();

    installed_pkgs.insert(
        installed_pkgs.end(),
        local_installed_pkgs.begin(), local_installed_pkgs.end()
    );

    auto &depends = info["Depends"];
    depends.append(info["MakeDepends"]);
    if (!depends.isArray()) { return false; }

    auto extract_dep_name = [](const std::string &dep) -> std::string {
        static const std::regex versioned_dep_regex(R"(([^<>=]+))");
        std::smatch match;
        if (std::regex_search(dep, match, versioned_dep_regex)) {
            return match.str(1);
        }
        return dep;
    };

    for (const auto &dep : depends) {
        const std::string full_dep = dep.asString();
        const std::string dep_name = extract_dep_name(full_dep);

        bool found = false;

        for (auto *pkg : installed_pkgs) {
            if (dep_name == alpm_pkg_get_name(pkg)) {
                found = true;
                break;
            }

            alpm_list_t *provides = alpm_pkg_get_provides(pkg);
            for (auto *p = provides; p != nullptr; p = p->next) {
                auto *prov = static_cast<alpm_depend_t *>(p->data);

                if (dep_name == prov->name) {
                    found = true;
                    break;
                }
            }

            if (found) break;
        }

        if (!found) { return true; }
    }

    return false;
}


void
Tab::get_installed_pkgs()
{
    auto local_pkgs = m_aur_client->get_locally_installed_pkgs();
    m_card_data.installed_pkgs->reserve(local_pkgs.size());

    for (auto *pkg : local_pkgs) {
        m_card_data.installed_pkgs->emplace_back(
            alpm_pkg_get_name(pkg), alpm_pkg_get_version(pkg)
        );
    }
}