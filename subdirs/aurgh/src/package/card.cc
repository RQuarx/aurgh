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

#include <gtkmm/linkbutton.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <giomm/file.h>
#include <json/value.h>

#include <utility>

#include "package/client.hh"
#include "package/card.hh"
#include "utils.hh"
#include "data.hh"

using pkg::Card;


Card::Card(json            pkg,
           const CardData &card_data,
           str_view        icon_path) :
    m_card_data(card_data),
    m_package(std::move(pkg)),
    m_button_dimmed(false)
{
    builder_t b = Gtk::Builder::create_from_file(card_data.card_builder_file);

#if GTK4
    m_card             = b->get_widget<Gtk::Box>("card_box");
    m_action_button    = b->get_widget<Gtk::Button>("action_button");
    m_version_label    = b->get_widget<Gtk::Label>("version_label");
    m_desc_label       = b->get_widget<Gtk::Label>("description_label");
    m_name_link        = b->get_widget<Gtk::LinkButton>("name_link");
    m_name_label       = b->get_widget<Gtk::Label>("name_label");
    m_outofdate_label  = b->get_widget<Gtk::Label>("outofdate_label");
    m_popularity_label = b->get_widget<Gtk::Label>("popularity_label");
    m_votes_label      = b->get_widget<Gtk::Label>("votes_label");
    m_package_icon     = b->get_widget<Gtk::Image>("package_image");

    set_child(*m_card);
    set_valign(Gtk::Align::START);
    set_halign(Gtk::Align::FILL);
#else
    b->get_widget("card_box", m_card);
    b->get_widget("action_button", m_action_button);
    b->get_widget("version_label", m_version_label);
    b->get_widget("description_label", m_desc_label);
    b->get_widget("name_link", m_name_link);
    b->get_widget("name_label", m_name_label);
    b->get_widget("outofdate_label", m_outofdate_label);
    b->get_widget("popularity_label", m_popularity_label);
    b->get_widget("votes_label", m_votes_label);
    b->get_widget("package_image", m_package_icon);

    add(*m_card);
    set_valign(Gtk::ALIGN_START);
    set_halign(Gtk::ALIGN_FILL);
#endif

    set_vexpand(false);
    set_hexpand(true);

    setup(str(icon_path));
}


auto
Card::signal_action_pressed(
    ) -> sigc::signal<void (pkg::Type, bool, const json &)>
{ return m_signal; }


auto
Card::setup(const str &icon_path) -> bool
{
    set_margin_start(5);
    set_margin_end(0);

    str
        pkg_desc    = m_package["Description"].asString(),
        pkg_version = m_package["Version"    ].asString(),
        pkg_name    = m_package["Name"       ].asString(),
        url         = m_package["URL"        ].asString(),
        markup      = "<big>{}</big>",
        icon_name;

    m_desc_label->set_label(pkg_desc   );

    if (m_package["OutOfDate"] != json::null) {
        m_outofdate_label->set_label("(Out-of-date)");
    }

    m_name_label->set_markup(utils::format(markup, pkg_name));
    m_name_link ->set_tooltip_text(url);
    m_name_link ->set_uri(url);

    markup = "<b>{}</b>";

    uint32_t votes      = m_package["NumVotes"].asUInt();
    float    popularity =
        std::roundf(m_package["Popularity"].asFloat() * 100) / 100;

    m_version_label   ->set_markup(utils::format(markup, pkg_version));
    m_popularity_label->set_markup(utils::format(markup, popularity ));
    m_votes_label     ->set_markup(utils::format(markup, votes      ));

    str_pair    pkg    = { pkg_name, pkg_version };
    int8_t      result = find_package(pkg);

    if      (result == -1) { icon_name = "document-save-symbolic"; }
    else if (result == 0 ) { icon_name = "edit-delete-symbolic";   }
    else                   { icon_name = "view-refresh-symbolic";  }

    m_action_button->set_image_from_icon_name(icon_name);
    m_action_button->signal_clicked().connect(sigc::bind(
        sigc::mem_fun(*this, &Card::on_button_clicked),
        pkg::Type(result), pkg_name
    ));

    try {
        m_package_icon->set(icon_path);
#if GTK4
        m_package_icon->set_icon_size(Gtk::IconSize::LARGE);
#else
        m_package_icon->set_pixel_size(Gtk::ICON_SIZE_LARGE_TOOLBAR);
#endif
    } catch (const Glib::Error &e) {
        data::logger->log(
            Logger::Warn,
            "Icon path not found, using default icon name "
            "\"package-x-generic-symbolic\""
        );
#if GTK4
        m_package_icon->set_from_icon_name("package-x-generic-symbolic");
        m_package_icon->set_icon_size(Gtk::IconSize::LARGE);
#else
        m_package_icon->set_from_icon_name("package-x-generic-symbolic",
                                           Gtk::ICON_SIZE_LARGE_TOOLBAR);
#endif
    }

    return true;
}


void
Card::on_button_clicked(pkg::Type result, const str &pkg_name)
{
    auto vec = m_card_data.actions->at(result);

    if (m_button_dimmed) {
        auto it = std::ranges::find(*vec, pkg_name);

        if (it != vec->end()) {
            vec->erase(it);
            m_action_button->set_opacity(ACTIVE_OPACITY);
            m_button_dimmed = false;
        }

        m_signal.emit(result, false, m_package);
    } else {
        vec->push_back(pkg_name);
        m_action_button->set_opacity(INACTIVE_OPACITY);
        m_button_dimmed = true;

        m_signal.emit(result, true, m_package);
    }
}


auto
Card::find_package(const str_pair &package) -> int8_t
{
    if (data::pkg_client->find_pkg(package.first) != nullptr) {
        return 0;
    }
    return -1;
    // if (utils::find(*m_installed_pkgs, package)) return 0;
    // for (const auto &[pkg_name, _] : *m_installed_pkgs) {
        // if (pkg_name == package.first) return 1;
    // }
    // return -1;
}


void
Card::refresh()
{
    for (auto t : { pkg::Update, pkg::Install, pkg::Remove }) {
        auto action = m_card_data.actions->at(t);
        auto pkg    = m_package["Name"].asString();
        if (!m_button_dimmed && utils::find(*action, pkg)) {
            m_button_dimmed = true;
            m_action_button->set_opacity(INACTIVE_OPACITY);
            return;
        }
    }

    m_button_dimmed = false;
    m_action_button->set_opacity(ACTIVE_OPACITY);
}